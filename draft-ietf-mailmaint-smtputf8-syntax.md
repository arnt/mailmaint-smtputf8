---
stand_alone: true
ipr: trust200902
cat: std
submissiontype: IETF
area: Applications
wg: mailmaint

docname: draft-ietf-mailmaint-smtputf8-syntax

title: SMTPUTF8 address syntax
lang: en
kw:
  - email
author:
- name: Arnt Gulbrandsen
  org: ICANN
  street: 6 Rond Point Schumann, Bd. 1
  city: Brussels
  code: 1040
  country: Belgium
  email: arnt@gulbrandsen.priv.no
- name: Jiankang Yao
  org: CNNIC
  street: No.4 South 4th Zhongguancun Street
  city: Beijing
  code: 100190
  country: China
  email: yaojk@cnnic.cn

normative:
  RFC2119:
  RFC5322:
  RFC6365:
  RFC6530:
  RFC6532:
  RFC6533:

informative:
  RFC3490:
  RFC5891:
  RFC6854:
  RFC6858:
  UAX24:
    target: https://unicode.org/reports/tr24
    title: Unicode Script Property
    author:
      -
        name: Ken Whistler
	email: ken@unicode.org
  UMLAUT:
    target: https://en.wikipedia.org/wiki/Metal_umlaut
    title: Metal Umlaut
  TYPE=EMAIL:
    title: WHATWG input type=email
    target: https://html.spec.whatwg.org/multipage/input.html#email-state-(type=email)

--- abstract

This document specifies rules for email addresses that are flexible
enough to express the addresses typically used with SMTPUTF8, while
avoiding confusing or risky elements.

This is one of a pair of documents: This is simple to implement,
contains only globally viable rules and is intended to be usable for
software such an MTA. Its companion defines has more complex rules,
takes regional usage into account and aims to allow only addresses
that are readable and cut-and-pastable in some community.

--- middle

# Introduction

{{RFC6530}}-{{RFC6533}} and {{RFC6854}}-{{RFC6858}} extend various
aspects of the email system to support non-ASCII both in localparts
and domain parts. In addition, some email software supports unicode in
domain parts by using encoded domain parts in the SMTP transaction
("RCPT TO:<info@xn--dmi-0na.fo>") and presenting the unicode version
(dømi.fo in this case) in the user interface.

The email address syntax extension is in {{RFC6532}}, and allows
almost all UTF8 strings as localparts. While this certainly allows
everything users want to use, it is also flexible enought to allow
many things that users and implementers find surprising and sometimes
worrying.

The flexibility has caused considerable reluctance to support the full
syntax in contexts such as web form address validation.

This document attempts to describe rules that:

1. includes the addresses that users generally want to use for
themselves and organizations want to provision for their employees.

2. excludes things that have been described as security risks.

3. Looks safe at first glance to implementers (including ones with
little unicode expertise) and are fairly easy to use in unit tests.

4. Contain no regional rules.

These goals are somewhat aspirational.

# Requirements Language

{::boilerplate bcp14-tagged}

# Terminology

Script, in this document, refers to the unicode script property (see
{{UAX24}}). Each code point is assigned to one script ("a" is Latin),
except that some are assigned to "Common" or a few other special
values. Fraktur and /etc/rc.local aren't scripts in this document, but
Latin is.

Latin refers those code points that have the script property "Latin"
in Unicode. Orléans in France and Münster in Germany both have Latin
names in this document. It also refers to combinations of those code
points and combining characters, and to strings that contain no code
points from other scripts.

Han, Cyrillic etc. refer to those code points that have the respective
script property in Unicode, as well as to strings that contain no code
points from other scripts.

ASCII refers to the first 128 code points within unicode, which
includes the letters A-Z but not É or Ü. It also refers to strings
that contain only ASCII code points.

Non-ASCII refers to unicode code points except the first 128, and also
to strings that contain at least one such code point.

By way of example, the address info@dømi.fo is latin and non-ASCII,
its localpart is latin and ASCII, and its domain part is latin and
non-ASCII. 中国 is a Han string in this document, but 阿Q正传 is
neither a Latin string nor a Han string, because it contains a Latin Q
and three Han code points.

# Rules

Based on the above goals, the following rules are formulated:

1. An address MUST NOT contain an a-label (e.g. xn--dmi-0na).

2. An address MUST contain only code points in the PRECIS
IdentifierClass.

3. An address MUST consist entirely of a sequence of composite
characters, ZWJ and ZWNJ. ("c" followed by "combining hook below" is
an example of a composite character, "d" is another example; see
{{RFC6365}} for the definition.)

4. An address MOT NOT contain more than one script, disregarding
ASCII. (Disregarding ASCII, the word Orléans contains only an é, which
is one script, namely Latin.)

# Examples

example@example.com is permitted, because 1) it does not contain any
a-label, 2) it consists entirely of permissible code points, 4) it
consists of 19 composite characters, and 4) it contains no non-ASCII
code points at all.

The address dømi@dømi.fo is permitted, because 1) it does not contain any
a-label, 2) does not apply, 3) it consists entirely of permissible
code points, 4) it consists of 12 composite characters, 5) does not apply
and 6) it consists entirely of 'Latin' and 'Common' code points (and
./@).

The address U+200E '@' U+200F '.' U+200E is not permitted, because 4)
U+200E and U+200F are not parts of composite characters.

阿Q正传@阿Q正传.example is permitted because it contains ASCII and Han,
dømi@dømi.fo is legal because it contains ASCII and Latin, but
阿Q正传@dømi.fo is illegal becasue it contains Han 阿 and the Latin
non-ASCII letter ø.

TODO: add more examples and rationales again.

# IANA Considerations {#IANA}

This document does not require any actions from the IANA.

# Security Considerations {#SECURITY}

When a program renders a unicode string on-screen or audibly and
includes a substring supplied by a potentially malevolent source, the
included substring can affect the rendering of a surprisingly large
part of the overall string.

This document describes rules that make it difficult for an attacker
to use email addresses for such an attack. Implementers should be
aware of other possible vectors for the same kind of attack, such as
subject fields and email address display-names.

If an address is signed using DKIM and (against the rules of this
document) mixes left-to-right and right-to-left writing, parts of both
the localpart and the domain part can be rendered on the same side of
the '@'. This can create the appearance that a different domain signed
the message.

The rules in this document permit a number of code points that can
make it difficult to cut and paste.

--- back

# Acknowledgments

The authors wish to thank John C. Klensin, [your name here, please]
[oh wow, the ack section is already outdated]

Dømi.fo and 例子.中国 are reserved by nic.fo and CNNIC for use in
examples and documentation.

阿Q正传@ is a famous Chinese novella, 阿Q is the main character.

# Instructions to the RFC editor

Please remove all mentions of the Protocol Police before publication
(including this sentence).

Please remove the Open Issues section.

# Open issues

1. PRECIS IdentifierClass?

2. More examples.

3. Wording to identify destiny; I think this should probably become a
proposed standard and modify a couple of RFCs, but I'm uncertain about
some details and left that open now.

4. More words on the relationship between this and the
companion. There are several parallel differences, maybe this warrants
a section of its own.

5. Should this even mention the requirements placed on domains by
IDNA, ICANN, web browsers and others?
