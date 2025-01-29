#include "stm32f4xx.h"
#include "timeBase.h"


int main(void)
{
  int i = 0;

  timeBase_init();

  /* Infinite loop */
  while (1)
  {
    i++;
  }
}
