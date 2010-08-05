%option reentrant noyywrap case-insensitive
%option extra-type="com::wavii::pfp::tokenizer::tokenizer_out *"
%{

#include <pfp/tokenizer.h>

%}

SGML      <\/?[A-Za-z!][^>]*>
SPMDASH   &(MD|mdash);|\x96|\x97|\xe2\x80\x93|\xe2\x80\x94
SPAMP     &amp;
SPPUNC    &(HT|TL|UR|LR|QC|QL|QR|odq|cdq|lt|gt|#[0-9]+);
SPLET     &[aeiouAEIOU](acute|grave|uml);
SPACE     [ \t]+
SPACENL   [ \t\r\n]+
SENTEND   [ \t\n][ \t\n]+|[ \t\n]+([A-Z]|{SGML})
DIGIT     [0-9]
DATE      {DIGIT}{1,2}[\-\/]{DIGIT}{1,2}[\-\/]{DIGIT}{2,4}
NUM       {DIGIT}+|{DIGIT}*([.:,]{DIGIT}+)+
NUMBER    [\-+]?{NUM}|\({NUM}\)
/* Constrain fraction to only match likely fractions */
FRAC      ({DIGIT}{1,4}[- ])?{DIGIT}{1,4}\\?\/{DIGIT}{1,4}
FRAC2     \xc2\xbc|\xc2\xbd|\xc2\xbe
DOLSIGN   ([A-Z]*\$|#)
DOLSIGN2  \xc2\xa2|\xc2\xa3|\xc2\x80|\xe2\x82\xac
/* not used DOLLAR  {DOLSIGN}[ \t]*{NUMBER}  */
/* |\( ?{NUMBER} ?\))  # is for pound signs */
WORD      ([A-Za-z]|\xc3[\x80-\xbf]|{SPLET})+
/* The $ was for things like New$ */
/* WAS: only keep hyphens with short one side like co-ed */
/* But treebank just allows hyphenated things as words! */
THING     [A-Za-z0-9]+([_-][A-Za-z0-9]+)*
THINGA    [A-Z]+(([+&]|{SPAMP})[A-Z]+)+
THING3    [A-Za-z0-9]+(-[A-Za-z]+){0,2}(\\?\/[A-Za-z0-9]+(-[A-Za-z]+){0,2}){1,2}
APOS      [']|\xc2\x92|\xe2\x80\x99|&apos;
HTHING    ([A-Za-z0-9][A-Za-z0-9%.,]*(-([A-Za-z0-9]+|{ACRO}\.))+)|[dDOlL]{APOS}{THING}
REDAUX    {APOS}([msdMSD]|re|ve|ll)
/* For things that will have n't on the end. They can't end in 'n' */
SWORD     [A-Za-z]*[A-MO-Za-mo-z]
SREDAUX   n{APOS}t
/* Tokens you want but already okay: C'mon 'n' '[2-9]0s '[eE]m 'till?
   [Yy]'all 'Cause Shi'ite B'Gosh o'clock.  Here now only need apostrophe
   final words. */
APOWORD   {APOS}n{APOS}?|[lLdDjJ]'|Dunkin{APOS}|somethin{APOS}|ol{APOS}|{APOS}em|C{APOS}mon|{APOS}[2-9]0s|{APOS}till?|o{APOS}clock|[A-Za-z][a-z]*[aeiou]{APOS}[aeiou][a-z]*|{APOS}cause
FULLURL   https?:\/\/[^ \t\n\f\r\"<>|()]+[^ \t\n\f\r\"<>|.!?(){},-]
LIKELYURL ((www\.([^ \t\n\f\r\"<>|.!?(){},]+\.)+[a-zA-Z]{2,4})|(([^ \t\n\f\r\"`'<>|.!?(){},-_$]+\.)+(com|net|org|edu)))(\/[^ \t\n\f\r\"<>|()]+[^ \t\n\f\r\"<>|.!?(){},-])?
EMAIL     [a-zA-Z0-9][^ \t\n\f\r\"<>|()]*@([^ \t\n\f\r\"<>|().]+\.)+[a-zA-Z]{2,4}

/* Abbreviations - induced from 1987 WSJ by hand */
ABMONTH   Jan|Feb|Mar|Apr|Jun|Jul|Aug|Sep|Sept|Oct|Nov|Dec
/* Jun and Jul barely occur, but don't seem dangerous */
ABDAYS    Mon|Tue|Tues|Wed|Thu|Thurs|Fri
/* In caseless, |a\.m|p\.m handled as ACRO, and this is better as can often
   be followed by capitalized. */
/* Sat. and Sun. barely occur and can easily lead to errors, so we omit them */
ABSTATE   Calif|Mass|Conn|Fla|Ill|Mich|Pa|Va|Ariz|Tenn|Mo|Md|Wis|Minn|Ind|Okla|Wash|Kan|Ore|Ga|Colo|Ky|Del|Ala|La|Nev|Neb|Ark|Miss|Vt|Wyo|Tex
ACRO      [A-Za-z](\.[A-Za-z])+|(Canada|Sino|Korean|EU|Japan|non)-U\.S|U\.S\.-(U\.K|U\.S\.S\.R)
ABTITLE   Mr|Mrs|Ms|Miss|Drs?|Profs?|Sens?|Reps?|Lt|Col|Gen|Messrs|Govs?|Adm|Rev|Maj|Sgt|Pvt|Mt|Capt|St|Ave|Pres
ABPTIT    Jr|Bros|Sr
ABCOMP    Inc|Cos?|Corp|Pty|Ltd|Plc|Bancorp|Dept|Mfg|Bhd|Assn
ABNUM     Nos?|Prop|Ph
/* p used to be in ABNUM list, but it can't be any more, since the lexer
   is now caseless.  We don't want to have it recognized for P.  Both
   p. and P. are now under ABBREV4. ABLIST also went away as no-op [a-e] */
/* ABBREV1 abbreviations are normally followed by lower case words.  If
   they're followed by an uppercase one, we assume there is also a
   sentence boundary */
ABBREV3   {ABMONTH}|{ABDAYS}|{ABSTATE}|{ABCOMP}|{ABNUM}|{ABPTIT}|etc|ft
ABBREV1   {ABBREV3}\.

/* ABRREV2 abbreviations are normally followed by an upper case word.  We
   assume they aren't used sentence finally */
/* ACRO Is a bad case -- can go either way! */
ABBREV4   [A-Za-z]|{ABTITLE}|vs|Alex|Cie|a\.k\.a|TREAS|{ACRO}
ABBREV2   {ABBREV4}\.
/* Cie. is used before French companies */
/* in the WSJ Alex. is generally an abbreviation for Alex. Brown, brokers! */
/* In tables: Mkt. for market Div. for division of company, Chg., Yr.: year */

PHONE     \([0-9]{3}\)\ ?[0-9]{3}[\- ][0-9]{4}
OPBRAC    [<\[]
CLBRAC    [>\]]
HYPHENS   \-+|(\xe8\x88\x92)+
LDOTS     \.{3,5}|(\.\ ){2,4}\.|\xc2\x85|\xe2\x80\xa6
ATS       @+
UNDS      _+
ASTS      \*+|(\\\*){1,3}
HASHES    #+
FNMARKS   {ATS}|{HASHES}|{UNDS}
INSENTP   [,;:]
QUOTES    `|{APOS}|``|''|(\xe2\x80\x98|\xe2\x80\x99|\xe2\x80\x9c|\xe2\x80\x9d|\xc2\x91|\xc2\x92|\xc2\x93|\xc2\x94){1,2}
DBLQUOT   \"|&quot;
TBSPEC    -(RRB|LRB|RCB|LCB|RSB|LSB)-|C\.D\.s|D'Amico|M'Bow|pro-|anti-|S&P-500|Jos\.|cont'd\.?|B'Gosh|S&Ls|N'Ko|'twas
TBSPEC2   {APOS}[0-9][0-9]

%%

{SGML}                    { yyextra->put(yytext); }
{SPMDASH}                 { yyextra->put("--"); }
{SPAMP}                   { yyextra->put("&"); }
{SPPUNC}                  { yyextra->put(yytext); }
{WORD}/{REDAUX}           { yyextra->put(yytext); }
{SWORD}/{SREDAUX}         { yyextra->put(yytext); }
{WORD}                    { yyextra->put_american(yytext); }
{APOWORD}                 { yyextra->put(yytext); }
{FULLURL}                 { yyextra->put(yytext); }
{LIKELYURL}               { yyextra->put(yytext); }
{EMAIL}                   { yyextra->put(yytext); }
{REDAUX}/[^A-Za-z]        { yyextra->put_cp1252(yytext); }
{SREDAUX}                 { yyextra->put_cp1252(yytext); }
{DATE}                    { yyextra->put(yytext); }
{NUMBER}                  { yyextra->put(yytext); }
{FRAC}                    { yyextra->put_escape(yytext, '/'); }
{FRAC2}                   { yyextra->put_cp1252(yytext); }
{TBSPEC}                  { yyextra->put(yytext); }
{THING3}                  { yyextra->put_escape(yytext, '/'); }
{DOLSIGN}                 { yyextra->put(yytext); }
{DOLSIGN2}                { yyextra->put_cp1252(yytext); }
{ABBREV1}/{SENTEND}       { yyextra->put(yytext);  /* TODO: reinstate this when i can figure out how to get flex/jflex case-insensitivity to be the same: unput('.'); */  /* return a period for next time */ }
{ABBREV1}                 { yyextra->put(yytext); }
{ABBREV2}                 { yyextra->put(yytext); }
{ABBREV4}/{SPACE}         { yyextra->put(yytext); }
{ACRO}/{SPACENL}          { yyextra->put(yytext); }
{TBSPEC2}/{SPACENL}       { yyextra->put(yytext); }
{WORD}\./{INSENTP}        { yyextra->put(yytext); }
{PHONE}                   { yyextra->put(yytext); }
{DBLQUOT}/[A-Za-z0-9$]    { yyextra->put("``"); }
{DBLQUOT}                 { yyextra->put("''"); }
\+                        { yyextra->put(yytext); }
%|&                       { yyextra->put(yytext); }
\~|\^                     { yyextra->put(yytext); }
\||\\|0x7f                {}
{OPBRAC}                  { yyextra->put("-LRB-"); }
{CLBRAC}                  { yyextra->put("-RRB-"); }
\{                        { yyextra->put("-LCB-"); }
\}                        { yyextra->put("-RCB-"); }
\(                        { yyextra->put("-LRB-"); }
\)                        { yyextra->put("-RRB-"); }
{HYPHENS}                 { if (yyleng >= 3 && yyleng <= 4) yyextra->put("--"); else yyextra->put(yytext); }
{LDOTS}                   { yyextra->put("..."); }
{FNMARKS}                 { yyextra->put(yytext); }
{ASTS}                    { yyextra->put_escape(yytext, '*'); }
{INSENTP}                 { yyextra->put(yytext); }
\.|\?|\!                  { yyextra->put(yytext); }
=                         { yyextra->put(yytext); }
\/                        { yyextra->put_escape(yytext, '/'); }
{HTHING}/[^a-zA-Z0-9.+]   { yyextra->put(yytext); }
{THING}                   { yyextra->put(yytext); }
{THINGA}                  { yyextra->put_amp(yytext); }
'[A-Za-z].                { yyextra->put("`"); yyless(1); /* invert quote - using trailing context didn't work.... */ }
{REDAUX}                  { yyextra->put_cp1252(yytext); }
{QUOTES}                  { yyextra->put_cp1252(yytext); }
\0|{SPACE}                { }
\n|\r|\r\n                { }
&nbsp;                    { }
.                         { yyextra->err(yytext); }

%%

void com::wavii::pfp::tokenizer::tokenize(const std::string & in, std::vector<std::string> & out) const
{
  yyscan_t scanner;
  tokenizer_out to(out, *this);
  yylex_init_extra( &to, &scanner );
  yy_scan_string(in.c_str(), scanner);
  yylex(scanner);
  yylex_destroy(scanner);
}