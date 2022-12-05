#include "../include/rserrors.h"

#include <iostream>

const detail::RsErrorCodeCategory &errorCodeCategory()
{
  static detail::RsErrorCodeCategory c;
  return c;
}

const detail::RsErrorConditionCategory &errorConditionCategory()
{
  static detail::RsErrorConditionCategory c;
  return c;
}