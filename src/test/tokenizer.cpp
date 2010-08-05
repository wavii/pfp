#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <pfp/tokenizer.h>

using namespace com::wavii::pfp;

struct tokenizer_test_fixture
{
  std::vector<std::string> words;
  tokenizer t;
  tokenizer_test_fixture() : t("./etc/americanizations") {}
};

BOOST_AUTO_TEST_SUITE( tokenizer_test )

BOOST_FIXTURE_TEST_CASE( test_nothing, tokenizer_test_fixture )
{
  t.tokenize("", words);
  BOOST_CHECK_EQUAL(words.size(), 0);
}

BOOST_FIXTURE_TEST_CASE( test_sgml, tokenizer_test_fixture )
{
  t.tokenize("<blah>", words);
  BOOST_REQUIRE_EQUAL(words.size(), 1);
  BOOST_REQUIRE_EQUAL(words[0], "<blah>");
}

BOOST_FIXTURE_TEST_CASE( test_mdash, tokenizer_test_fixture )
{
  t.tokenize("&mdash;", words);
  BOOST_REQUIRE_EQUAL(words.size(), 1);
  BOOST_REQUIRE_EQUAL(words[0], "--");
  words.clear(); t.tokenize("\x96", words);
  BOOST_REQUIRE_EQUAL(words.size(), 1);
  BOOST_REQUIRE_EQUAL(words[0], "--");
  words.clear(); t.tokenize("\xe2\x80\x93", words);
  BOOST_REQUIRE_EQUAL(words.size(), 1);
  BOOST_REQUIRE_EQUAL(words[0], "--");
}

BOOST_FIXTURE_TEST_CASE( test_amp, tokenizer_test_fixture )
{
  t.tokenize("hey &Amp; dude", words);
  BOOST_REQUIRE_EQUAL(words.size(), 3);
  BOOST_REQUIRE_EQUAL(words[1], "&");
  words.clear(); t.tokenize("b&b", words);
  BOOST_REQUIRE_EQUAL(words.size(), 1);
  BOOST_REQUIRE_EQUAL(words[0], "b&b");
  words.clear(); t.tokenize("b&Amp;b", words);
  BOOST_REQUIRE_EQUAL(words.size(), 1);
  BOOST_REQUIRE_EQUAL(words[0], "b&b");
}

BOOST_FIXTURE_TEST_CASE( test_cant, tokenizer_test_fixture )
{
  t.tokenize("I CAN'T GET NO", words);
  BOOST_REQUIRE_EQUAL(words.size(), 5);
  BOOST_REQUIRE_EQUAL(words[1], "CA");
  BOOST_REQUIRE_EQUAL(words[2], "N'T");
}

BOOST_FIXTURE_TEST_CASE( test_dangit, tokenizer_test_fixture )
{
  t.tokenize("dang'it", words);
  BOOST_REQUIRE_EQUAL(words.size(), 3);
  BOOST_REQUIRE_EQUAL(words[1], "`");
}

BOOST_FIXTURE_TEST_CASE( test_americanize, tokenizer_test_fixture )
{
  t.tokenize("haematologists devour the colour superleukaemia.", words);
  BOOST_REQUIRE_EQUAL(words.size(), 6);
  BOOST_REQUIRE_EQUAL(words[0], "hematologists");
  BOOST_REQUIRE_EQUAL(words[1], "devour");
  BOOST_REQUIRE_EQUAL(words[3], "color");
  BOOST_REQUIRE_EQUAL(words[4], "superleukemia");
}

BOOST_FIXTURE_TEST_CASE( test_escape, tokenizer_test_fixture )
{
  t.tokenize("this / that", words);
  BOOST_REQUIRE_EQUAL(words.size(), 3);
  BOOST_REQUIRE_EQUAL(words[1], "\\/");
  words.clear(); t.tokenize("this \\/ that", words);
  BOOST_REQUIRE_EQUAL(words.size(), 3);
  BOOST_REQUIRE_EQUAL(words[1], "\\/");
}

BOOST_FIXTURE_TEST_CASE( test_period, tokenizer_test_fixture )
{
  t.tokenize("Bob, Inc. bought a monkey.", words);
  BOOST_REQUIRE_EQUAL(words.size(), 7);
  BOOST_REQUIRE_EQUAL(words[2], "Inc.");
  BOOST_REQUIRE_EQUAL(words[3], "bought");
  // TODO: get these tests to run when I can figure out how to get flex/jflex to play with eachother regarding case insensitivity
  /*words.clear(); t.tokenize("Bob, Inc. Bought a monkey.", words);
  BOOST_REQUIRE_EQUAL(words.size(), 8);
  BOOST_REQUIRE_EQUAL(words[2], "Inc.");
  BOOST_REQUIRE_EQUAL(words[3], ".");*/
}

BOOST_AUTO_TEST_SUITE_END()
