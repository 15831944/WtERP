#include "Money"

#include "Wt/WException"
#include "Wt/WStringStream"
#include "Wt/WLocale"

#include <boost/lexical_cast.hpp>
//#include <boost/range/iterator_range.hpp>

namespace Wt {
  namespace Payment {

Money::Money():
  valueInCents_(0),
  currency_("")
{}

Money::Money(long long value, int cents, const std::string& currency):
  currency_(currency)
{
  valueInCents_ = (value * 100) + cents;
}

Money::Money(long long valueInCents, const std::string &currency):
  valueInCents_(valueInCents),
  currency_(currency)
{

}

Money::Money(const std::string &str, const std::string& currency):
  currency_(currency)
{
	setValueFromString(str);
}

const std::string Money::toString() const
{
  WStringStream ans;
  if(cents() > 9)
    ans << value() << "." << cents(); //todo use formater.
  else
    ans << value() << ".0" << cents(); //todo use formater.

  return ans.str();
}

void Money::setValueFromString(const std::string &str)
{
	const WLocale &locale = WLocale::currentLocale();
	size_t dotPos = str.find(".");
	if(dotPos == std::string::npos)
		valueInCents_ = WLocale::currentLocale().toLong(str) * 100;
	else
	{
		long long left = WLocale::currentLocale().toLong(str.substr(0, dotPos));
		unsigned int right = boost::lexical_cast<unsigned int>(str.substr(dotPos + 1));
		while(right > 99)
		{
			right /= 10;
		}
		valueInCents_ = (left * 100) + right;
	}
}

void Money::checkCurrency(Money& ans, const Money& v1, const Money& v2)
{
  if(v1.currency() == "" && v1.valueInCents() != 0){
    throw WException("Payment::Money::checkCurrency "
                     "money with no currency has value.");
  }

  if(v2.currency() == "" && v2.valueInCents() != 0){
    throw WException("Payment::Money::checkCurrency "
                     "money with no currency has value.");
  }

  if(v1.currency() == ""){
    ans.setCurrency(v2.currency());
    return;
  }

  if(v2.currency() == ""){
    ans.setCurrency(v1.currency());
    return;
  }

  if(v1.currency() != v2.currency()){
    throw WException("Payment::Money::checkCurrency different currency");
  }
}

Money& Money::operator= (const Money& money)
{
  valueInCents_ = money.valueInCents();
  currency_ = money.currency();

  return (*this);
}

Money& Money::operator+= (const Money& money)
{
  checkCurrency(*this, *this, money);

  valueInCents_ += money.valueInCents();

  return (*this);
}

Money& Money::operator-= (const Money& money)
{
  checkCurrency(*this, *this, money);

  valueInCents_ -= money.valueInCents();

  return (*this);
}

Money& Money::operator*= (double value)
{
  valueInCents_*=value;

  return (*this);
}

Money& Money::operator/= (double value)
{
  valueInCents_/=value;

  return (*this);
}

Money& Money::operator*= (unsigned value)
{
  valueInCents_*=value;

  return (*this);
}

Money& Money::operator/= (unsigned value)
{
  valueInCents_/=value;

  return (*this);
}

#ifndef WT_TARGET_JAVA
Money operator+ (const Money& v1, const Money& v2)
{
  Money ans = v1;
  ans += v2;
  return ans;
}

Money operator- (const Money& v1, const Money& v2)
{
  Money ans = v1;
  ans -= v2;
  return ans;
}

Money operator* (const Money& v1, double v2)
{
  Money ans = v1;
  ans *= v2;
  return ans;
}

Money operator* (double v1, const Money& v2)
{
  Money ans = v2;
  ans *= v1;
  return ans;
}

Money operator/ (const Money& v1, double v2)
{
  Money ans = v1;
  ans /= v2;
  return ans;
}
#endif

  }
}
