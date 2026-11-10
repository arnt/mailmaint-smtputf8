---
stand_alone: true
ipr: trust200902
cat: std
updates: 6532
submissiontype: IETF
area: "Applications and Real-Time"
wg: mailmaint

docname: draft-ietf-mailmaint-smtputf8-syntax-latest

title: SMTPUTF8 Email Addresses
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
  RFC5890:
  RFC5892:
  RFC6531:
  RFC6532:
  RFC8264:
  UAX15:
    target: https://www.unicode.org/reports/tr15
    title: Unicode Normalization Forms
    date: 2026-08-22
    author:
      -
        name: Ken Whistler
  UAX24:
    target: https://unicode.org/reports/tr24
    title: Unicode Script Property
    date: 2025-07-31
    author:
      -
        name: Ken Whistler

informative:
  RFC952:
  RFC5321:
  RFC5891:
  RFC6530:
  IDN-TABLES:
    target: https://www.iana.org/domains/idn-tables
    title: Repository of IDN Practices
    author:
      -
        org: IANA

--- abstract

This document updates the email address syntax to include the
addresses in active use, include the addresses likely to be used based
on what writing systems are used together, and exclude addresses that
pose risks when recipient software renders or processes them.

--- middle

# Introduction

{{RFC6531}} extends SMTP and {{RFC6532}} extends message syntax to
support non-ASCII both in localparts and domain parts. In addition,
some email software supports Unicode in domain parts by using encoded
domain parts in the SMTP transaction ("RCPT TO:<info@xn--dmi-0na.fo>")
and presenting the Unicode version (dømi.fo in this case) in the user
interface.

The email address syntax extension in {{RFC6532}} allows almost all
UTF8 strings as localparts. While this certainly allows everything
users want to use, it is also flexible enough to allow many things
that users and implementers find surprising and sometimes worrying.
For example, the domain extracted by code may not match that shown to
users on-screen.

This document defines rules that allow the addresses currently used
and the addresses that users and implementers have asked for. It
disallows addresses that cause confusing rendering, including many
instances of homographic rendering. It does not try to disallow
anything that is already disallowed by IDNA (see {{RFC5891}}) or
by common registry policies (see {{IDN-TABLES}}).

# Requirements Language

{::boilerplate bcp14-tagged}

# Terminology

Script, in this document, refers to the Unicode script property (see
{{UAX24}}). Each code point is assigned to one script ("a" is Latin),
except that some are assigned to "Common" or a few other special
values. "Latin" and "Cyrillic" are examples of scripts.

Latin refers to those code points that have the script property "Latin"
in Unicode. Orléans in France and Münster in Germany both have Latin
names in this document. It also refers to combinations of those code
points and combining characters, and to strings that contain no code
points from other scripts.

Han, Cyrillic etc. refer to those code points that have the respective
script property in Unicode, as well as to strings that contain no code
points from other scripts.

ASCII refers to the first 128 code points within Unicode, which
includes the letters A-Z but not É or Ü. It also refers to strings
that contain only ASCII code points.

Non-ASCII refers to Unicode code points except the first 128, and also
to strings that contain at least one such code point.

By way of example, the address info@dømi.fo is Latin and non-ASCII,
its localpart is Latin and ASCII, and its domain part is Latin and
non-ASCII. 中国 is a Han string in this document, but 阿Q正传 is
neither a Latin string nor a Han string, because it contains a Latin Q
and three Han code points.

The term A-label is defined in {{RFC5890}} section 2.3.2.1.

PRECIS is described in {{RFC8264}}, and the IdentifierClass in section
4.2 of that document.

# Rules

The following four rules apply to the
{{I-D.ietf-emailcore-rfc5322bis}} mailbox production, as indirectly
extended by {{RFC6532}}.

1. An atom in an address MUST NOT be an A-label (e.g. xn--dmi-0na).

2. An address MUST contain only code points that the PRECIS
IdentifierClass treats as valid, and the U+0020 space. Where the
IdentifierClass requires a contextual rule, the address MUST satisfy
that rule (defined in {{RFC8264}} and described in depth in
{{RFC5892}} Appendix A).

3. An address MUST NOT contain more than one script, when ASCII is
disregarded. Han, Hiragana and Katakana count as one script here,
since Japanese uses all three together; so do Han and Hangul, and Han
and Bopomofo. (For example: In the word Orléans, Orl and ans are ASCII
and é is non-ASCII. Disregarding ASCII leaves a single é, necessarily
a single script, so the word contains only one script. Another
example: まり子 is Hiragana and Han, which count as one.)

4. An address MUST be in the Unicode NFC form (see {{UAX15}}).

These rules do not affect rules in other documents. Most notably,
domain-names in email addresses are almost always also domain names on
the public internet, which implies that the rules specified by
{{RFC952}}, {{RFC5891}} and others affect email addresses. Domain
registries often have policies regarding which names may be
registered.

These rules constrain how to generate email addresses in email
messages. This document does not require receivers to reject, alter
or normalize non-conforming addresses.

# Examples

example@example.com is permitted, because 1) it does not contain any
A-label, 2) it consists entirely of permissible code points, 3) it
contains no non-ASCII code points at all and 4) it is in NFC form.

The address dømi@dømi.fo is permitted, because 1) it does not contain
any A-label, 2) the IdentifierClass permits every code point in it, 3)
it consists entirely of 'Latin' and 'Common' code points and 4) it is
in NFC form.

The address U+200E '@' U+200F '.' U+200E is not permitted, because 2)
the IdentifierClass disallows U+200E and U+200F as ignorable code
points.

The address 'admin' U+3164 '@example.com' is not permitted for the
same reason. U+3164 HANGUL FILLER renders as nothing at all, so the
address looks exactly like admin@example.com. U+0640 ARABIC TATWEEL is
disallowed too, although it is a letter by category: it only elongates
the preceding letter, and would let one address be spelled in any
number of ways.

The address まり子@まり子.日本 is permitted, because 1) it contains no
A-label, 2) the IdentifierClass permits every code point in it, 3)
Hiragana and Han count as one script and 4) it is in NFC form.

阿Q正传@阿Q正传.example is permitted because it contains ASCII and Han,
dømi@dømi.fo is legal because it contains ASCII and Latin, but
阿Q正传@dømi.fo is not, because it contains both Han (阿) and Latin
non-ASCII (ø).

example@-example-.example.com is permitted by the rules above.
However, since {{RFC952}} and its successors prohibit both of the
hyphens, that email address is effectively not permitted on the
internet.

# Old stored messages

This section is informative.

A mailstore may contain old messages that do not conform to the rules
of this document, or indeed any RFC. Two typical examples may be To:
info@xn--dmi-0na.fo and To: =?ISO-8859-1?Q?info@d=F8mi.fo?=. A more
confusing example is "ali" U+202E "moc.elpmaxe@kravdraa.ec", which could
be rendered as alice.aardvark@example.com (U+202E changes display
direction). It is advisable to be careful about how to display an
address to users if that address does not conform to the rules in this
document.

# Relationships to other RFCs

This section is informative.

This document does not update {{RFC6531}}. This is because the ABNF in
{{RFC6531}} permits everything this document needs, and the
restrictions in this document are not needed within SMTP. (For
example, while a human recipient might be confused by a homograph
attack or a hard-to-read A-label, a receiving SMTP server can hardly
be.)

# IANA Considerations {#IANA}

This document does not require any actions from the IANA.

# Security Considerations {#SECURITY}

When a program renders a non-ASCII email address string on-screen or
stores it as a string (e.g. as part of a line of text in a system log
file), the address eventually shown to a human may not be as desired.
One possible way to diminish the risk is to replace impermissible code
points with U+FFFD, REPLACEMENT CHARACTER. There are also other
options. Note that changing an entire email message in this way risks
invalidating signatures such as DKIM.

The rules in this document permit a number of code points that can
make an address difficult to cut and paste.

{{RFC5321}} section 4.1.2 advises those who define mailboxes to avoid
case-sensitive localparts, and {{RFC6530}} section 10.1 advises them to
support other normalization forms of the same localpart. {{RFC8264}}
section 5.2.3 discusses some options for case mapping. When processing
addresses in remote domains, one cannot be sure which conversion rules
apply. Implementers may want to consider unusual cases such as the
Turkish dotless ı.

--- back

# Acknowledgments

The authors wish to thank John C. Klensin,
Daniel Eggert, Andrew Gallagher, Jeffrey Stedfast, Pete Resnick,
Narendra Singh Bisht,
(your name here, please)

(Wow, the ack section is already outdated)

Dømi.fo and 例子.中国 are reserved by nic.fo and CNNIC for use in
examples and documentation.

阿Q正传 is a famous Chinese novella, 阿Q is the main character.

# Instructions to the RFC editor

Please remove the Open Issues section.

This document uses the form U+0020 to refer to Unicode code points.
Please change that to the form most commonly used in newish RFCs — or
leave it if there is no dominant form. I'm too lazy to count today.

# Testing

This is a set of test addresses in JSON format.

Below is a verbatim copy of
https://github.com/arnt/mailmaint-smtputf8/blob/main/tests.json as it was
on (date here). It contains a number of strange and unusual code points, so
cutting and pasting this may not work. Rather, it is recommended to
either use the rfcstrip tool or download the tests using a command
such as curl
https://raw.githubusercontent.com/arnt/mailmaint-smtputf8/main/tests.json
> tests.json.

Note to IETF reviewers: The tests will be included here shortly before
publication (and after IETF Last Call).

# Open issues

1. SHOULD NOT or MUST NOT contain A-labels?
