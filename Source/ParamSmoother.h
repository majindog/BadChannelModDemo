#pragma once

class ParamSmoother
{
public:
    
    void prepare (float initVal, int nSamples)
    {
        currentL = initVal;
        currentR = initVal;
        target = initVal;
        numSamples = nSamples;
    }
    
    void setInc (int channel)
    {
        if (channel == 0)
            incL = (target - currentL) / numSamples;
        else
            incR = (target - currentR) / numSamples;
    }
    
    void loopCheck (int channel)
    {
        if (channel == 0 && target != currentL)
            currentL += incL;
        else if (channel != 0 && target != currentR)
            currentR += incR;
    }
    
    void outCheck()
    {
        if (target != currentL)
            currentL = target;

        if (target != currentR)
            currentR = target;
    }
    
    void update (float newTarget)
    {
        target = newTarget;
        incL = (target - currentL) / numSamples;
        incR = (target - currentR) / numSamples;
    }
    
    float getCurrent (int channel)
    {
        if (channel == 0)
            return currentL;
        else
            return currentR;
    }
    
private:
    
    float currentL = 0.0f;
    float currentR = 0.0f;
    float target = 0.0f;
    float incL = 0.0f;
    float incR = 0.0f;
    int numSamples = 0;
    
};
