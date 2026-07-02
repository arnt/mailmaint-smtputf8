---
stand_alone: true
ipr: trust200902
cat: std
submissiontype: IETF
area: "Applications and Real-Time"
wg: mailmaint

docname: draft-ietf-mailmaint-smtputf8-syntax-latest

title: Deciding Whether to Display an SMTPUTF8 Email Address
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
  I-D.ietf-emailcore-rfc5322bis:
  RFC5892:
  RFC6365:
  RFC6530:
  RFC6532:
  RFC8264:

informative:
  RFC3490:
  RFC5891:
  RFC6533:
  RFC6854:
  RFC6858:
  UAX24:
    target: https://unicode.org/reports/tr24
    title: Unicode Script Property
    date: 2025-07-31
    author:
      -
        name: Ken Whistler
  UMLAUT:
    target: https://en.wikipedia.org/wiki/Metal_umlaut
    title: Metal Umlaut
  TYPE=EMAIL:
    title: WHATWG input type=email
    target: https://html.spec.whatwg.org/multipage/input.html#email-state-(type=email)

--- abstract

This document recommends a best current practice for software that
displays an email address obtained from a potentially malevolent
source, such as the sender of an email message. It defines a set of
rules, simple enough to require little code, that exclude addresses
whose display could mislead the reader. An address that satisfies the
rules is safe to display in its native Unicode form, while one that
does not should be shown in a safer form.

This is one of a pair of documents. This one is simple to implement
and contains only globally viable rules. Its companion has more
complex rules, takes regional usage into account, and describes
addresses that can be read by some community and cut-and-pasted in
some locale.

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
everything users want to use, it is also flexible enough to allow
many things that users and implementers find surprising and sometimes
worrying.

Displaying such an address verbatim can be risky, because a possibly
hostile sender chooses it. For example, the sender might register
kravdraa.ec and send mail from ali U+202E moc.elpmaxe@kravdraa.ec,
which a trustful receiver might display on-screen as
alice.aardvark@example.com, with perfect DKIM/DMARC alignment.

This document describes rules that identify the addresses that are
safe to display, and recommends that software decline to display the
rest in their native form. The rules —

1. include the addresses that users generally want to use for
themselves and that organizations want to provision for their
staff;

2. exclude things that have been described as security risks;

3. are visibly safe to implementers (including ones with little
Unicode expertise) and are easy to use in unit tests;

4. contain no regional rules, for simplicity.

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


Based on the above goals, the following rules identify an address as
safe to display. An address is safe only if it satisfies all three.

1. An atom in an address MUST NOT be an a-label (e.g. xn--dmi-0na).

2. An address MUST contain only code points in the "A", "H" and "K"
classes defined by [RFC5892] and [RFC8264], as well as the code points
allowed by the "F" class, also defined by [RFC5892], "." and "@". (A
contains letters and digits, H contains join controls, K contains
ASCII and F contains a few exceptions.)

3. An address MUST NOT contain more than one script, when ASCII is
disregarded. (For example: In the word word Orléans, Orl and ans are
ASCII and é is non-ASCII. Since é is a single letter, the word
contains only one script.)

# Examples

example@example.com is permitted, because 1) it does not contain any
a-label, 2) it consists entirely of permissible code points and 3) it
contains no non-ASCII code points at all.

The address dømi@dømi.fo is permitted, because 1) it does not contain
any a-label, 2) it consists entirely of code points in the "A" and "K"
classes and 3) it consists entirely of 'Latin' and 'Common' code
points (and ./@).

The address U+200E '@' U+200F '.' U+200E is not permitted, because 3)
U+200E and U+200F are in the "C" class as defined by [RFC5892], not A/H/K/F.

阿Q正传@阿Q正传.example is permitted because it contains ASCII and Han,
dømi@dømi.fo is legal because it contains ASCII and Latin, but
阿Q正传@dømi.fo is illegal because it contains both Han (阿) and Latin
non-ASCII (ø).

# Displaying an address from an untrusted source {#practice}

Software that displays an email address obtained from a potentially
malevolent source faces the risks described in {{SECURITY}}. The
originator fields of an inbound message are the common case, but the
same applies wherever an untrusted address is displayed to human
users: address books populated from incoming mail, quoted text, search
results and logs shown in a user interface.

An address that satisfies the rules above MAY be displayed in its
native Unicode form.

An address that does not satisfy the rules SHOULD NOT be displayed in
its native Unicode form. The software SHOULD instead display it in a
form that cannot mislead the reader, such as the all-ASCII a-label form
of {{RFC5890}} for the domain, or with the offending code points shown
in an unambiguous escaped notation.

This practice applies only to addresses from untrusted sources.

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

This is why {{practice}} recommends against displaying a
non-conforming address in its native form: the rules are the line
between an address that a reader can trust at a glance and one that a
hostile sender may have designed.

The rules in this document permit a number of code points that can
make it difficult to cut and paste.

--- back

# Testing

This is a set of test addresses in JSON format.

Below is a verbatim copy of
https://github.com/arnt/mailmaint-smtputf8/tests.json as it was
on (date here). It contains a number of strange and unusual code points, so
cutting and pasting this may not work. Rather, it is recommended to
either use the rfcstrip tool or download the tests using a command
such as curl
https://github.com/arnt/mailmaint-smtputf8/tests.json >
tests.json.

Note to IETF reviewers: The tests will be included here shortly before
publication (and after IETF Last Call).

# Acknowledgments

The authors wish to thank John C. Klensin, (your name here, please)

(Wow, the ack section is already outdated)

Dømi.fo and 例子.中国 are reserved by nic.fo and CNNIC for use in
examples and documentation.

阿Q正传@ is a famous Chinese novella, 阿Q is the main character.

# Instructions to the RFC editor

Please remove all mentions of the Protocol Police before publication
(including this sentence).

Please remove the Open Issues section.

# Open issues

1. Wording to identify destiny; I think this should probably become a
proposed standard and modify a couple of RFCs, but I'm uncertain about
some details and left that open now.

4. More words on the relationship between this and the
companion. There are several parallel differences, maybe this warrants
a section of its own.

5. Should this even mention the requirements placed on domains by
IDNA, ICANN, web browsers and others?
