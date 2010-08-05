#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <pfp/state_list.hpp>
#include <pfp/unary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/binary_grammar.hpp>
#include <pfp/pcfg_parser.hpp>

using namespace com::wavii::pfp;

BOOST_AUTO_TEST_SUITE( pcfg_parser_test )

BOOST_AUTO_TEST_CASE( test_pcfg_parser )
{
  state_list states("./share/pfp/states");
  unary_grammar ug(states, "./share/pfp/unary_rules");
  binary_grammar bg(states, "./share/pfp/binary_rules");
  pcfg_parser pcfg(states, ug, bg);
  std::vector< std::vector< state_score_t > > sentence;

  {
    std::ifstream in("./etc/test/sample_input");
    std::string line;
    float f;
    while (std::getline(in, line))
    {
      std::istringstream iss(line);
      std::vector< state_score_t > word;
      state_score_t ss;
      while (iss >> ss.state >> f)
      {
        ss.score = static_cast<score_t>(f * consts::score_resolution);
        word.push_back(ss);
      }
      std::sort(word.begin(), word.end());
      sentence.push_back(word);
    }
  }

  node result;
  workspace ws(sentence.size(), states.size());
  BOOST_REQUIRE_EQUAL( pcfg.parse(sentence, ws, result), true );
  BOOST_REQUIRE_EQUAL( result.state, consts::goal_state );
  BOOST_REQUIRE_EQUAL( result.children.size(), 2 );
  BOOST_CHECK_EQUAL( result.children[1]->state, consts::boundary_state );
  BOOST_CHECK_EQUAL( states[result.children[0]->state].tag, "S^ROOT-v" );
}

BOOST_AUTO_TEST_SUITE_END()
