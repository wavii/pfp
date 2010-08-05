#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <pfp/state_list.hpp>
#include <pfp/lexicon.hpp>

using namespace com::wavii::pfp;

struct lexicon_test_fixture
{
  std::vector< std::pair< state_t, float > > scores;
  state_list states;
  lexicon lex;
  lexicon_test_fixture() : states("./share/pfp/states"), lex(states, "./share/pfp/words", "./share/pfp/sigs", "./share/pfp/word_state", "./share/pfp/sig_state") {}
};

BOOST_AUTO_TEST_SUITE( lexicon_test )

BOOST_FIXTURE_TEST_CASE( test_nothing, lexicon_test_fixture )
{
  lex.score("", std::back_inserter(scores));
  BOOST_CHECK_EQUAL(scores.size(), 18);
}

// known word, no smoothing
BOOST_FIXTURE_TEST_CASE( test_The, lexicon_test_fixture )
{
  lex.score("The", std::back_inserter(scores));
  BOOST_REQUIRE_EQUAL(scores.size(), 5);
  BOOST_CHECK_EQUAL( scores[0].first, 151 );
  BOOST_CHECK_CLOSE( scores[0].second, -7.8324527f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[1].first, 303 );
  BOOST_CHECK_CLOSE( scores[1].second, -5.7462034f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[2].first, 901 );
  BOOST_CHECK_CLOSE( scores[2].second, -2.4547340f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[3].first, 2220 );
  BOOST_CHECK_CLOSE( scores[3].second, -0.3566749f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[4].first, 5801 );
  BOOST_CHECK_CLOSE( scores[4].second, -1.4586150f, 0.0001f );
}

// known word, smoothing
BOOST_FIXTURE_TEST_CASE( test_promotional, lexicon_test_fixture )
{
  lex.score("promotional", std::back_inserter(scores));
  BOOST_REQUIRE_EQUAL(scores.size(), 131);
  BOOST_CHECK_EQUAL( scores[0].first, 13 );
  BOOST_CHECK_CLOSE( scores[0].second, -16.316783905029297f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[23].first,  125);
  BOOST_CHECK_CLOSE( scores[23].second, -14.719925880432129f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[36].first,  169);
  BOOST_CHECK_CLOSE( scores[36].second, -8.140656471252441f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[130].first,  11337);
  BOOST_CHECK_CLOSE( scores[130].second, -12.640483856201172f, 0.0001f );
}

// unknown word, found sig
BOOST_FIXTURE_TEST_CASE( test_phalanxes, lexicon_test_fixture )
{
  lex.score("phalanxes", std::back_inserter(scores));
  BOOST_REQUIRE_EQUAL(scores.size(), 18);
  BOOST_CHECK_EQUAL( scores[0].first, 77 );
  BOOST_CHECK_CLOSE( scores[0].second, -11.172365188598633f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[4].first, 137 );
  BOOST_CHECK_CLOSE( scores[4].second, -11.44262981414795f, 0.0001f );
  BOOST_CHECK_EQUAL( scores[17].first, 833 );
  BOOST_CHECK_CLOSE( scores[17].second, -20.891258239746094f, 0.0001f );

}

BOOST_AUTO_TEST_SUITE_END()
