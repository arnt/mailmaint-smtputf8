#!/usr/bin/env python3

# Diffs the repertoire that rule 2 of draft-ietf-mailmaint-smtputf8-syntax
# permits against UAX31's XID_Start/XID_Continue, and against the full
# PRECIS IdentifierClass. The permitted repertoire is read out of the
# tables the C++ example uses, so this checks the same data the example
# does, not a second implementation of the rules.
#
# Usage: uax31diff.py [DerivedCoreProperties.txt]
#
# The last report is a regression check on the tables rather than a
# comparison: it should stay empty, since precisdata generates them
# from the IdentifierClass.
# Get that file from https://www.unicode.org/Public/UCD/latest/ucd/ .
# Its Unicode version, the version behind unicodedata-[12].h and the one
# in python's unicodedata module should ideally all be the same;
# differences confined to recently added scripts are version skew.

import os, re, sys, unicodedata
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__)) + '/c/'

# The F-class code points the example permits only in context, and the
# H-class join controls rule 2 permits outright.
CONTEXTUAL = ({0xB7, 0x375, 0x5F3, 0x5F4, 0x30FB} |
              set(range(0x660, 0x66A)) | set(range(0x6F0, 0x6FA)))
JOIN_CONTROLS = {0x200C, 0x200D}

# RFC5892 2.6, the exceptions that are DISALLOWED although the category
# rules would have made them PVALID.
F_DISALLOWED = {0x640, 0x7FA, 0x302E, 0x302F, 0x303B} | set(range(0x3031, 0x3036))

# Noncharacters, the other half of PrecisIgnorableProperties (M).
NONCHARACTERS = (set(range(0xFDD0, 0xFDF0)) |
                 {p * 0x10000 + o for p in range(17) for o in (0xFFFE, 0xFFFF)})

# RFC5892 2.9, Hangul_Syllable_Type L, V and T.
OLD_HANGUL_JAMO = (set(range(0x1100, 0x1200)) | set(range(0xA960, 0xA97D)) |
                   set(range(0xD7B0, 0xD7C7)) | set(range(0xD7CB, 0xD7FC)))

def draft_repertoire():
    """The code points rule 2 permits, per unicodedata-[12].h, where a
    script of None means the code point is in none of A, H, K and F."""
    script = {}
    supplement = [l.strip().rstrip(',') for l in open(HERE + 'unicodedata-2.h')
                  if not l.startswith('/')]
    for line in open(HERE + 'unicodedata-1.h'):
        m = re.match(r'\{ (0x[0-9a-f]+), (\d+), (\w+), (-?\d+) \}', line)
        if not m:
            continue
        start, length, s, index = (int(m.group(1), 16), int(m.group(2)),
                                   m.group(3), int(m.group(4)))
        for i in range(length):
            script[start + i] = s if index < 0 else supplement[index + i]
    return ({cp for cp, s in script.items() if s != 'None'} |
            {0x20} |            # rule 2 adds SPACE
            JOIN_CONTROLS)      # class H

def derived(path, wanted):
    props = {name: set() for name in wanted}
    for line in open(path):
        fields = [f.strip() for f in line.split('#')[0].split(';')]
        if len(fields) < 2 or fields[1] not in props:
            continue
        first, _, last = fields[0].partition('..')
        props[fields[1]].update(range(int(first, 16), int(last or first, 16) + 1))
    return props

def identifierclass_rejects(cp, ignorable=frozenset()):
    """RFC8264 4.2.3 disallows these regardless of the category rules,
    so a class-by-class reading of rule 2 lets them back in. ASCII is
    grandfathered by the ASCII7 category and never lands here."""
    if cp <= 0x7E:
        return None
    if cp in F_DISALLOWED:
        return 'F exception, DISALLOWED'
    if cp in OLD_HANGUL_JAMO:
        return 'OldHangulJamo (I)'
    if cp in ignorable and cp not in JOIN_CONTROLS:
        return 'PrecisIgnorableProperties (M)'
    if unicodedata.normalize('NFKC', chr(cp)) != chr(cp):
        return 'HasCompat (Q)'
    return None

def ranges_of(cps):
    out, cps = [], sorted(cps)
    for cp in cps:
        if out and out[-1][1] == cp - 1:
            out[-1][1] = cp
        else:
            out.append([cp, cp])
    return out

def name(cp):
    return unicodedata.name(chr(cp), '<unnamed>')

def report(title, cps, limit=12):
    print('\n== %s: %d code points ==' % (title, len(cps)))
    cats = Counter(unicodedata.category(chr(cp)) for cp in cps)
    print('   categories: ' + ', '.join('%s=%d' % c for c in cats.most_common()))
    rs = ranges_of(cps)
    for a, b in rs[:limit]:
        if a == b:
            print('   U+%04X %s' % (a, name(a)))
        else:
            print('   U+%04X..U+%04X (%d) %s ..' % (a, b, b - a + 1, name(a)))
    if len(rs) > limit:
        print('   ... and %d more ranges' % (len(rs) - limit))

props = derived(sys.argv[1] if len(sys.argv) > 1 else 'DerivedCoreProperties.txt',
                ('XID_Start', 'XID_Continue', 'Default_Ignorable_Code_Point'))
xid_start, xid_cont = props['XID_Start'], props['XID_Continue']
ignorable = props['Default_Ignorable_Code_Point'] | NONCHARACTERS
draft = draft_repertoire()

unassigned = {cp for cp in range(0x110000)
              if unicodedata.category(chr(cp)) == 'Cn'}

# Rule 2 makes no start/continue distinction, so XID_Continue is the
# comparable set; the start/continue difference gets its own report.
report('permitted here, not XID_Continue', draft - xid_cont - unassigned)
report('XID_Continue, not permitted here',
       xid_cont - draft - CONTEXTUAL - unassigned)
report('permitted here, XID_Continue but not XID_Start',
       (draft & xid_cont) - xid_start)
report('permitted in context here (F class), and XID_Continue',
       CONTEXTUAL & xid_cont)
report('permitted in context here (F class), not XID_Continue',
       CONTEXTUAL - xid_cont)

rejected = {cp: identifierclass_rejects(cp, ignorable) for cp in sorted(draft)}
rejected = {cp: why for cp, why in rejected.items() if why}
print('\n== permitted here, disallowed by the PRECIS IdentifierClass: '
      '%d code points ==' % len(rejected))
for why, n in Counter(rejected.values()).most_common():
    sample = [cp for cp in rejected if rejected[cp] == why][:4]
    print('   %-30s %5d  e.g. %s' %
          (why, n, ', '.join('U+%04X %s' % (cp, name(cp)[:30]) for cp in sample)))
