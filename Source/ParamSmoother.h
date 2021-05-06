#pragma once

class ParamSmoother
{
public:
    
    void prepare (float initVal, float initScalar)
    {
        valLeft = initVal;
        valRight = initVal;
        target = initVal;
        scalar = initScalar;
    }
    
    void loopCheck (int channel)
    {
        if (channel == 0)
            valLeft = valLeft - scalar * (valLeft - target);
        else
            valRight = valRight - scalar * (valRight - target);
    }
    
    float getCurrent (int channel)
    {
        if (channel == 0)
            return valLeft;
        else
            return valRight;
    }
    
    void update (float newTarget)
    {
        target = newTarget;
    }
    
private:
    
    float valLeft;
    float valRight;
    float scalar;
    float target;
    
};
