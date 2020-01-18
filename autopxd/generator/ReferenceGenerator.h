#pragma once

#include <string>
#include "IGenerator.h"

class ReferenceGenerator : public IGenerator
{
private:
    std::string str;

public:
    ReferenceGenerator() = default;
    virtual ~ReferenceGenerator() = default;

    virtual std::string GetType() const noexcept override
    {
        return "reference: ";
    };
    
    virtual std::string GetString() override
    {
        return str;
    };
    
    virtual void SetString(std::string str) override
    {
        this->str = str;
    };
};

