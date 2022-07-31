// Build date
// Project property -> Settings -> Build step -> Step before build -> Command : "rm -rf ../Debug/Core/Src/user/builddate.o"

/* Include system header files -----------------------------------------------*/
#include <stdio.h>

/* Include user header files -------------------------------------------------*/
#include "builddate.h"

/* Exported macros -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Imported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported functions prototypes----------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
char *getBuildDate(void)
{
	static char strBuildDate[25] = {'\0'};
	static unsigned char once = 1;
	if(once != 0)
	{
		once = 0;
		sprintf(strBuildDate, "%s %s", __DATE__, __TIME__);     // ex. "Feb 13 2019 13:23:21"
	}
	return strBuildDate;
}
