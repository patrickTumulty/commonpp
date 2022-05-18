//
// Created by Patrick Tumulty on 5/17/22.
//

#include "InputProcessor.h"

#include <utility>
#include "VectorUtils.h"
#include "StringUtils.h"

/**
 * Constructor
 *
 * @param processors ArgProcessor's
 */
InputProcessor::InputProcessor(std::vector<ArgProcessor> processors) : argProcessors(std::move(processors))
{
    // Empty
}

/**
 * Process inputs
 *
 * @param argc argc
 * @param argv argv
 * @return 0 is successful
 */
int InputProcessor::processInputs(int argc, char **argv)
{
    std::vector<std::string> result;
    std::vector<std::string> inputArgs;
    for (int i = 0; i < argc; i++)
    {
        inputArgs.emplace_back(argv[i]);
    }

    parseInputToArgProcessors(inputArgs);

    return processArguments();
}

/**
 * Parse the given input arguments into ArgProcessors
 *
 * @param inputArgs input arg vectors
 */
void InputProcessor::parseInputToArgProcessors(std::vector<std::string> &inputArgs)
{
    std::vector<std::string> clearList;

    for (ArgProcessor processor : argProcessors)
    {
        for (int i = 0; i < inputArgs.size(); i++)
        {
            std::string currentArg = inputArgs[i];

            if (processor.getArgName() == currentArg ||
               (StringUtils::startsWith(processor.getShortArgName(), currentArg) &&
               !processor.getShortArgName().empty()))
            {
                if (processor.isFlag())
                {
                    addProcessorToFlagQueue(currentArg, processor, clearList);
                }
                else
                {
                    addProcessorToActionQueue(inputArgs, i, clearList, processor);
                }

                VectorUtils::removeCommon(&inputArgs, clearList);

                break;
            }
        }
    }

    unusedInputArgs = inputArgs;
}

/**
 * Add ArgProcessor to the action queue
 *
 * @param inputArgs input argument vector
 * @param currentIndex the current input args index
 * @param clearList vector to stored used arguments (eventually cleared from inputArgs)
 * @param processor target ArgProcessor
 */
void InputProcessor::addProcessorToActionQueue(const std::vector<std::string> &inputArgs,
                                               int currentIndex,
                                               std::vector<std::string> &clearList,
                                               ArgProcessor &processor)
{
    std::vector<std::string> values;
    int numberOfValues = processor.isFinal() ?
                         (int) (inputArgs.size() - currentIndex - 1) :
                         processor.getExpectedNumberOfArgs();
    for (int j = 1; j < numberOfValues + 1; j++)
    {
        const std::string& a = inputArgs[currentIndex + j];
        values.push_back(a);
        clearList.push_back(a);
    }
    processor.setArgumentValues(values);
    clearList.push_back(inputArgs[currentIndex]);
    actionQueue.push(processor);
}

/**
 * Add ArgProcessor to flag queue
 *
 * @param currentArg the current input argument string
 * @param processor target ArgProcessor
 * @param clearList vector to stored used arguments (eventually cleared from inputArgs)
 */
void InputProcessor::addProcessorToFlagQueue(const std::string &currentArg,
                                             ArgProcessor &processor,
                                             std::vector<std::string> &clearList)
{
    processor.setArgumentValues(std::vector<std::string>({currentArg}));
    clearList.push_back(currentArg);
    flagQueue.push(processor);
}

/**
 * Process the flag and action queues
 *
 * @return
 */
int InputProcessor::processArguments()
{
    try
    {
        return doProcessArguments();
    }
    catch (...)
    {
        return -1;
    }
}

/**
 * Do process arguments
 *
 * @return 0 is successful
 */
int InputProcessor::doProcessArguments()
{
    while (!flagQueue.empty())
    {
        int ret = flagQueue.front().process();
        if (ret != 0)
        {
            return ret;
        }
        flagQueue.pop();
    }

    while (!actionQueue.empty())
    {
        int ret = actionQueue.front().process();
        if (ret != 0)
        {
            return ret;
        }
        actionQueue.pop();
    }

    return 0;
}

/**
 * Get the unused input args. The string arguments that weren't matched to, or captured by, an ArgProcessor
 *
 * @return string vector of unused input args
 */
const std::vector<std::string> &InputProcessor::getUnusedInputArgs() const
{
    return unusedInputArgs;
}
