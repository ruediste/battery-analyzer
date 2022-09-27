#include "resistorMatcher.h"
#if IS_FRAMEWORK_NATIVE
#include <math.h>
#include <vector>
#include <algorithm>

/*
The accuracy of the measurement/feedback resistors have a large inpact of the acutal current
produced by the battery tester. While the errors are calibrated and compensated in software,
there is a limit to the range that can be compensated. With 5% tolerance resistor, this range
is easily exceeded.

It is therefore cruicial, to either use 1% tolerance resistors, or to match the resistors. This
tool helps with the matching. To use it, take a some of resistors, measure their values and enter
the results into the table below. Also specify the number of channels you need resistors for.

To run the tool, start the simulator and hit 'r'.

In the code below, the following resistor are named as follows

ref -- -[R1]- - -[R2]- -- shunt
in  -- -[R3]- + -[R4]- -- bat
*/
namespace resistorMatcher
{

    // resistor values for the input resistors R1, R3 (22k) [kOhm]
    double rInputs[] = {21.71,21.88,21.82,21.70, 0};

    // resistor values of the feedback resistors R2, R4 (10k) [kOhm]
    double rFeedbacks[] = {9.94,9.92,9.93,9.93, 0};

    int desiredChannels = 2;

    // value of the shunt resistor [Ohm]
    double rShunt = 0.22;

    // Voltage of reference input
    double uRef = 1.65;

    // maximum input voltagge
    double uInMax = (uRef + 3.3) / 2;

    // minimum input voltage
    double uInMin = (uRef + 0) / 2;

    double uBatMin = 2.8;
    double uBatMax = 4.2;

    struct CalcResult
    {
        int i1;
        int i2;
        int i3;
        int i4;
        double iMin = NAN;
        double iMax = NAN;

        void mergeCurrent(double i)
        {
            if (i < 0 && (isnan(iMin) || i > iMin))
                iMin = i;
            if (i > 0 && (isnan(iMax) || i < iMax))
                iMax = i;
        }

        double r1() { return rInputs[i1]; }
        double r2() { return rFeedbacks[i2]; }
        double r3() { return rInputs[i3]; }
        double r4() { return rFeedbacks[i4]; }
        double ratio()
        {
            double result = (r1() / r2()) / (r3() / r4());
            if (result < 1)
                return 1 / result;
            return result;
        }
    };

    double calculateCurrent(int i1, int i2, int i3, int i4, double uBat, double uIn, WINDOW *w)
    {
        double r1 = rInputs[i1];
        double r2 = rFeedbacks[i2];
        double r3 = rInputs[i3];
        double r4 = rFeedbacks[i4];

        double uOpIn = uBat + (uIn - uBat) * (r4 / (r3 + r4));
        double I = (uRef - uOpIn) / r1;
        double uShunt = uOpIn - r2 * I;
        double current = (uShunt - uBat) / rShunt;

        // wprintw(w, "r1: %f, r2: %f, r3: %f, r4: %f, uBat: %f, uIn: %f, uOpIn: %f, I: %f, uShunt: %f, current: %f\n", r1, r2, r3, r4, uBat, uIn, uOpIn, I, uShunt, current);

        return current;
    }

    CalcResult calculate(int i1, int i2, int i3, int i4, WINDOW *w)
    {
        CalcResult result;
        result.i1 = i1;
        result.i2 = i2;
        result.i3 = i3;
        result.i4 = i4;
        result.mergeCurrent(calculateCurrent(i1, i2, i3, i4, uBatMax, uInMax, w));
        result.mergeCurrent(calculateCurrent(i1, i2, i3, i4, uBatMax, uInMin, w));
        result.mergeCurrent(calculateCurrent(i1, i2, i3, i4, uBatMin, uInMax, w));
        result.mergeCurrent(calculateCurrent(i1, i2, i3, i4, uBatMin, uInMin, w));
        return result;
    }

    std::vector<std::vector<CalcResult>> permute(int n, std::vector<CalcResult>::iterator begin, std::vector<CalcResult>::iterator end)
    {
        std::vector<std::vector<CalcResult>> result;
        for (std::vector<CalcResult>::iterator ptr = begin; ptr < end; ptr++)
        {
            if (n == 1)
            {
                std::vector<CalcResult> entry;
                entry.push_back(*ptr);
                result.push_back(entry);
            }
            else
                for (std::vector<CalcResult> tail : permute(n - 1, ptr + 1, end))
                {
                    bool duplicate = false;
                    for (CalcResult tailElement : tail)
                    {
                        if (tailElement.i1 == ptr->i1 || tailElement.i1 == ptr->i3)
                            duplicate = true;
                        if (tailElement.i2 == ptr->i2 || tailElement.i2 == ptr->i4)
                            duplicate = true;
                        if (tailElement.i3 == ptr->i3 || tailElement.i3 == ptr->i1)
                            duplicate = true;
                        if (tailElement.i4 == ptr->i4 || tailElement.i4 == ptr->i2)
                            duplicate = true;
                    }
                    if (duplicate)
                        continue;
                    std::vector<CalcResult> entry;
                    entry.push_back(*ptr);
                    entry.insert(entry.end(), tail.begin(), tail.end());
                    result.push_back(entry);
                }
        }
        return result;
    }

    void calculate(WINDOW *w)
    {
        std::vector<CalcResult> results;

        // do the full permutation
        for (int i1 = 0; rInputs[i1] != 0; i1++)
            for (int i2 = 0; rFeedbacks[i2] != 0; i2++)
                for (int i3 = 0; rInputs[i3] != 0; i3++)
                    for (int i4 = 0; rFeedbacks[i4] != 0; i4++)
                    {
                        if (i1 == i3 || i2 == i4)
                            continue;

                        results.push_back(calculate(i1, i2, i3, i4, w));
                    }

        // choose results
        std::vector<std::vector<CalcResult>> combinations = permute(desiredChannels, results.begin(), results.end());

        std::vector<CalcResult> bestCombination;
        double bestCombinationCurrent = 0;
        double bestCombinationRatio = INFINITY;
        for (std::vector<CalcResult> combination : combinations)
        {
            double minCurrent = INFINITY;
            double maxRatio = 0;
            for (CalcResult r : combination)
            {
                minCurrent = std::min(minCurrent, std::min(abs(r.iMax), abs(r.iMin)));
                maxRatio = std::max(maxRatio, r.ratio());
            }
            // wprintw(w, "MinCurrent: %f\n", minCurrent);
            // if (bestCombinationCurrent < minCurrent)
            if (bestCombinationRatio > maxRatio)
            {
                bestCombinationCurrent = minCurrent;
                bestCombinationRatio = maxRatio;
                bestCombination = combination;
            }
        }

        wprintw(w, "Best Combination (min absolute current: %f, maxRatio: %f):\n\n", bestCombinationCurrent, bestCombinationRatio);
        for (CalcResult result : bestCombination)
        {
            wprintw(w, "R10/R2: %f (%i) R12/R4: %f (%i) ", result.r1(), result.i1, result.r2(), result.i2);
            wprintw(w, "R11/R3: %f (%i) R13/R5: %f (%i) ", result.r3(), result.i3, result.r4(), result.i4);
            wprintw(w, "iMax: %f iMin: %f ratios: %f/%f => %f\n\n", result.iMax, result.iMin, result.r1() / result.r2(), result.r3() / result.r4(), result.ratio());
        }
        wrefresh(w);
        return;
    }
}
#endif