/*
    Copyright (c) 2015 Martin Buck
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
    OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
  * \brief example showing the usage of simulated annealing
  *        to find the minimum of x^2
  *        but now using 4 threads
  */

#include <iostream>
#include <cmath>

#include "cppOpt.h"

using namespace cppOpt;

template <typename T>
class MySolver : public OptSolverBase<T>
{
public:
    //define your own calculation
    void calculate(OptCalculation<T> &optCalculation) const
    {
        //defined x^2 as function to be optimised
        optCalculation.result = pow(optCalculation.get_parameter("X"),2);
    }
};

using namespace std;

int main()
{
    //setup the optimisation
    //define your boundaries
    //to enable 4 threads, the range of example_1 is split
    OptBoundaries<double>
            optBoundaries1,
            optBoundaries2,
            optBoundaries3,
            optBoundaries4;

    optBoundaries1.add_boundary(-5.0, -2.5, "X");
    optBoundaries2.add_boundary(-2.5, 0.0, "X");
    optBoundaries3.add_boundary(0.0, 2.5, "X");
    optBoundaries4.add_boundary(2.5, 5.0, "X");

    //instansiate your calculator
    MySolver<double> mySolver;

    //number of calculations
    unsigned int maxCalculations = 300;

    //we want to find the minimum
    OptTarget optTarget = MINIMIZE;

    //how fast the simulated annealing algorithm slows down
    //http://en.wikipedia.org/wiki/Simulated_annealing
    double coolingFactor = 0.9;

    //the chance in the beginning to follow bad solutions
    double startChance = 0.25;

    //create your optimiser
    //using simulated annealing
    //now creating 4 to run all at once
    OptSimulatedAnnealing<double> opt1(optBoundaries1,
                                       maxCalculations / 4,
                                       &mySolver,
                                       optTarget,
                                       0.0, //only required if approaching / diverging
                                       coolingFactor,
                                       startChance);

    OptSimulatedAnnealing<double> opt2(optBoundaries2,
                                       maxCalculations / 4,
                                       &mySolver,
                                       optTarget,
                                       0.0, //only required if approaching / diverging
                                       coolingFactor,
                                       startChance);

    OptSimulatedAnnealing<double> opt3(optBoundaries3,
                                       maxCalculations / 4,
                                       &mySolver,
                                       optTarget,
                                       0.0, //only required if approaching / diverging
                                       coolingFactor,
                                       startChance);

    OptSimulatedAnnealing<double> opt4(optBoundaries4,
                                       maxCalculations / 4,
                                       &mySolver,
                                       optTarget,
                                       0.0, //only required if approaching / diverging
                                       coolingFactor,
                                       startChance);

    //enable logging
    //boundaries object required to know the parameters names for the header
    OptBase<double>::enable_logging("example_simulated_annealing_x_square_threaded.log", optBoundaries1);

    //let's go
    OptBase<double>::run_optimisations();

    //print result
    OptCalculation<double> best = OptBase<double>::get_best_calculation(optTarget, 0.0);
    cout << best.to_string_header() << endl;
    cout << best.to_string_values() << endl;

    return 0;
}
