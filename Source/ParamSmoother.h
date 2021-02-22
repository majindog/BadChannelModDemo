#pragma once

class ParamSmoother
{
public:
    
    void prepare (float initVal)
    {
        current = initVal;
        target = initVal;
        localTarget = initVal;
    }
    
    void setParam (int numSamples)
    {
        localTarget = target;
        inc = (localTarget - current) / numSamples;
    }
    
    void loopCheck()
    {
        if (localTarget != current)
            current += inc;
    }
    
    void outCheck()
    {
        if (localTarget != current)
            current = localTarget;
    }
    
    void updateTarget (float newTarget)
    {
        target = newTarget;
    }
    
    float getCurrent()
    {
        return current;
    }
    
    float getTarget()
    {
        return target;
    }
    
private:
    
    float current = 0.0f;
    float target = 0.0f;
    float inc = 0.0f;
    float localTarget = 0.0f;
    
};
