pfp -- pretty fast statistical parser for natural languages
===========================================================

pfp is a **pretty fast statistical parser** for probabilistic context free grammars.  pfp uses the exhaustive [CYK](http://en.wikipedia.org/wiki/CYK_algorithm) algorithm found in [the Stanford NLP parser](http://nlp.stanford.edu/software/lex-parser.shtml) (and gratefully uses its trained grammar), but boasts the following improvements:

* 3-4x faster than the Stanford parser
* Uses 5-8x less resident memory
* Thread-safe/multi-core
* Python bindings
* Lightweight threadpool http server
* Command-line client

## Installing

The following works on **Ubuntu 10.04 LTS**:

    sudo apt-get install -y git-core cmake build-essential libboost-all-dev python-dev
    git clone http://github.com/wavii/pfp.git && cd pfp
    cmake .
    make
    ./test && sudo make install

To install the python library:

    sudo python setup.py install

If you are running OS X using [Homebrew](http://mxcl.github.com/homebrew/) you may need to first get your dependencies in order:

    brew install icu4c boost boost-jam
    ln -s /usr/local/Cellar/icu4c/50.1/include/unicode /usr/local/include/
    ln -s /usr/local/Cellar/icu4c/50.1/include/layout /usr/local/include/
    ls -1 /usr/local/Cellar/icu4c/50.1/lib/ | xargs -IFILE ln -s /usr/local/Cellar/icu4c/50.1/lib/FILE /usr/local/lib

## Benchmarks

Parse times and maximum memory usage were recorded on a c1.medium Amazon EC2 instance.  pfp is compared to Stanford Parser version **1.6.3** running on **sun-java6**.

	Sentence Length  Stanford (avg ms)  pfp (avg ms)   Stanford (res mb) pfp (res mb)
    0-9              21.0407725322      5.12853470437  83                10
    10-19            104.675603217      34.9076086957  83                14
    20-29            363.596421471      124.78         105               21
    30-39            830.084942085      320.664122137  150               30
    40-45            1607.43820225      542.533333333  150               35

## Usage

**pfpc** is a command-line client that reads from `stdin` and writes parses to `stdout`:

    $ echo "I love monkeys." | pfpc 2>/dev/null
    (ROOT (S (NP (PRP I)) (VP (VBP love) (NP (NNS monkeys))) (. .)) )

**pfpd** is a threadpool web server that wraps pfp:

    $ pfpd localhost 8080 2>/dev/null &
    [1] 3600
    $ curl http://localhost:8080/parse/I+love+monkeys.
    (ROOT (S (NP (PRP I)) (VP (VBP love) (NP (NNS monkeys))) (. .)) )

**pypfp** are python bindings for pfp:

    $ python
    >>> import pfp
    >>> pfp.Parser().parse("I love monkeys.")
    '(ROOT (S (NP (PRP I)) (VP (VBP love) (NP (NNS monkeys))) (. .)) )'

## License

(GPL v2)

Copyright (c) 2010 Wavii, Inc. <http://wavii.com/>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
