#include <glm.hpp>
#include <iostream>

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

class Polynomial
{
    private:

    void powerRule()
    {
        int deg = zerothDerivative.size() - 1;
        firstDerivative.clear();
        for (int i = 0; i < zerothDerivative.size() - 1; i++)
        {
            firstDerivative.push_back(zerothDerivative[i] * (deg - i));
        }
    }

    double func(const double x, const std::vector<double>& terms) const
    {
        double result = 0.0;
        for (size_t i = 0; i < terms.size(); i++)
            result = result * x + terms[i];
        return result;
    }

    public:
    
    std::vector<double> zerothDerivative;
    std::vector<double> firstDerivative;
    double tol;

    /**
     * @brief Polynomial root solver
     * 
     * @param terms: terms of the polynomial in descending degree
     * @param tolerance: the difference in results required to stop
     */
    Polynomial(const std::vector<double> terms, const double tolerance)
    {
        this->zerothDerivative = terms;
        powerRule();
        this->tol = tolerance;
    }

    double findRoot(double guess) const
    {
        double newGuess = 0.0;
        double& oldGuess = guess;
        while(true)
        {
            oldGuess = newGuess;
            double f = func(oldGuess, zerothDerivative);
            double fp = func(oldGuess, firstDerivative);
            if (std::abs(fp) < 1e-12)
                throw std::runtime_error("Evaluated derivative was 0.0");
            newGuess = oldGuess - (f / fp);
            if (std::abs(newGuess - oldGuess) < tol)
                break;
        }
        return guess;
    }
};

#endif