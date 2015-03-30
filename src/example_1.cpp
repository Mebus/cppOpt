#include <iostream>
#include <cmath> ///@todo only needed for example formula, maybe remove later

#include "CalculatorBase.h"
#include "OptSimulatedAnnealing.h"

using namespace cppOpt;

class MyCalculator : public CalculatorBase
{
public:
    //define your own calculation
    void calculate(OptValue &optValue) const
    {
        //defined x^2 as function to be optimized
        optValue.result = pow(optValue.get_parameter("X"),2);
    }
};

using namespace std;

int main()
{
    //setup the optimization
    //define your boundaries
    OptBoundaries optBoundaries;
    optBoundaries.add_boundary(-5.0, 5.0, "X");
    
    //instansiate your calculator
    MyCalculator myCalculator;

    //number of calculations
    unsigned int maxCalculations = 30;
    
    //we want to find the minimum
    OptTarget optTarget = MINIMIZE;
    
    //how fast the simulated annealing algorithm slows down
    //http://en.wikipedia.org/wiki/Simulated_annealing
    T coolingFactor = 0.995;
    
    //the chance in the beginning to follow bad solutions
    T startChance = 0.25;

    //create your optimizer
    //using simulated annealing
    OptSimulatedAnnealing opt(optBoundaries,
                              maxCalculations,
                              &myCalculator,
                              optTarget,
                              0.0, //only required if approaching / diverging
                              coolingFactor,
                              startChance);

    //enable logging
    //boundaries object required to know the parameters names for the header
    OptBase::enable_logging("log.test", optBoundaries);

    //let's go
    OptBase::run_optimisations();

    return 0;
}

