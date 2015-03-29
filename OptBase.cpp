#include "OptBase.h"

///@todo split this file into static and non-static

std::mutex
    OptBase::mutexQueueTodo,
    OptBase::mutexQueueCalculated,
    OptBase::mutexQueueFinished,
    OptBase::mutexPOptimizers;

std::queue< std::pair<OptValue, OptBase*> >
    OptBase::queueTodo,
    OptBase::queueFinished;

std::set<OptBase*>
    OptBase::pOptimizers;

//------------------------------------------------------------------------------

OptBase::OptBase(const OptBoundaries &optBoundaries,
                 unsigned int maxCalculations,
                 CalculatorBase* pCalculator,
                 OptTarget optTarget,
                 T targetValue) :
    maxCalculations(maxCalculations),
    optBoundaries(optBoundaries),
    pCalculator(pCalculator),
    optTarget(optTarget),
    targetValue(targetValue)
{
    mutexPOptimizers.lock();
    pOptimizers.insert(this);
    mutexPOptimizers.unlock();
    srand( time(NULL) + rand() ); ///@todo maybe only seed once (on static level)
}

//------------------------------------------------------------------------------

OptBase::~OptBase()
{
    mutexPOptimizers.lock();
    pOptimizers.erase( pOptimizers.find(this) );
    mutexPOptimizers.unlock();
}

//------------------------------------------------------------------------------

void OptBase::add_finished_calculation(OptValue optValue, OptBase* pOptBase)
{
    previousCalculations.push_back(optValue);

    mutexQueueFinished.lock();
    queueFinished.push({optValue, pOptBase});
    mutexQueueFinished.unlock();

    if(result_better(optValue, bestCalculation))
        bestCalculation = optValue;
}

//------------------------------------------------------------------------------

T OptBase::bad_value() const
{
    switch(optTarget)
    {
        case MINIMIZE:
            return std::numeric_limits<T>::max();

        case MAXIMIZE:
            return std::numeric_limits<T>::min();

        case APPROACH:
            if(targetValue > 0.0)
                return std::numeric_limits<T>::min();
            else
                return std::numeric_limits<T>::max();

        case DIVERGE:
            return targetValue;

        default: //MINIMIZE
            return std::numeric_limits<T>::max();
    }
}

//------------------------------------------------------------------------------

bool OptBase::result_better(const OptValue &result, const OptValue &other) const
{
    switch(optTarget)
    {
        case MINIMIZE:
            return result.result < other.result;

        case MAXIMIZE:
            return result.result > other.result;

        case APPROACH:
            return fabs(targetValue - result.result) < fabs(targetValue - other.result);

        case DIVERGE:
            return fabs(targetValue - result.result) > fabs(targetValue - other.result);

        default: //MINIMIZE
            return result.result < other.result;
    }
}

//------------------------------------------------------------------------------

void OptBase::run_optimisations(unsigned int maxThreads)
{
    //get the first to-calculate value of every optimizer
    //and push it onto the todo queue
    mutexPOptimizers.lock();
    for(const auto &pOptimizer : pOptimizers)
    {
        if(pOptimizer->previousCalculations.size() == 0)
            push_todo(pOptimizer->get_next_value(), pOptimizer);
    }
    mutexPOptimizers.unlock();

    std::vector <std::thread> threads; ///@todo vector best container type? (maybe list is enough)

    for (unsigned int i=0; i<maxThreads; ++i)
        threads.emplace_back(  std::thread( std::bind(&OptBase::threaded_work) )  );

    for (auto &thread :threads ) ///@todo add proper cooldown here (or counting variable)
        thread.join();
}

//------------------------------------------------------------------------------

unsigned int OptBase::number_optimizers()
{
    unsigned int out(0);
    mutexPOptimizers.lock();
    out = pOptimizers.size();
    mutexPOptimizers.unlock();
    return out;
}

//------------------------------------------------------------------------------

T OptBase::random_factor()
{
    return rand()/(T)(RAND_MAX);
}

//------------------------------------------------------------------------------

void OptBase::threaded_work()
{
    while(true) ///@todo there has to be a check whether there still are calculations
    {
        if(available_todo())
        {
            ///@todo possible race condition here. Make it that pop checks for availability itself (return bool and have the value be a reference which is changed bool pop(out) )
            ///@todo or add another mutex which only locks for this check mutex availabilityCheck
            std::pair <OptValue, OptBase*> todo = pop_todo();
            OptValue optValue = todo.first;
            OptBase* pOptBase = todo.second;

            pOptBase->pCalculator->calculate(optValue);

#ifdef DEBUG
            std::cout << optValue.to_string() << std::endl;
#endif

            pOptBase->add_finished_calculation(optValue, pOptBase); ///@todo this method should be static (or not need the pointer argument)

            if(pOptBase->previousCalculations.size() > pOptBase->maxCalculations) ///@todo maybe be >=
                break;

            //only add the next one if there still are more
            push_todo(pOptBase->get_next_value(), pOptBase);
        }



        std::this_thread::sleep_for(std::chrono::milliseconds(0)); ///@todo sleep time must be defineable by library user [maybe add as paramter to run_optimisations]
    }

}

//------------------------------------------------------------------------------

void OptBase::push_todo(OptValue optValue, OptBase* pOptBase)
{
    mutexQueueTodo.lock();
    queueTodo.push({optValue, pOptBase});
    mutexQueueTodo.unlock();
}

//------------------------------------------------------------------------------

void OptBase::push_finished(OptValue optValue, OptBase *pOptBase)
{
    mutexQueueFinished.lock();
    queueFinished.push({optValue, pOptBase});
    mutexQueueFinished.unlock();
}

//------------------------------------------------------------------------------

bool OptBase::available_todo()
{
    bool out(false);

    mutexQueueTodo.lock();
    out = !queueTodo.empty();
    mutexQueueTodo.unlock();

    return out;
}

//------------------------------------------------------------------------------

bool OptBase::available_finished()
{
    bool out(false);

    mutexQueueFinished.lock();
    out = !queueFinished.empty();
    mutexQueueFinished.unlock();

    return out;
}

//------------------------------------------------------------------------------

std::pair<OptValue, OptBase*> OptBase::pop_todo()
{
    mutexQueueTodo.lock();
    auto out = queueTodo.front();
    queueTodo.pop();
    mutexQueueTodo.unlock();
    return out;
}

//------------------------------------------------------------------------------

std::pair<OptValue, OptBase*> OptBase::pop_finished()
{
    mutexQueueFinished.lock();
    auto out = queueFinished.front();
    queueFinished.pop();
    mutexQueueFinished.unlock();
    return out;
}

//------------------------------------------------------------------------------

