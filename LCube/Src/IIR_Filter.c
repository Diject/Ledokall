
#include "IIR_Filter.h"

//http://jaggedplanet.com/iir/iir-explorer.asp

typedef float REAL;

#define NBQ 10
REAL biquada[]={0.9303591928745112,1.0387114255349439,0.8047361836665106,0.9931919133655572,0.6947900846920096,0.9730451716039531,0.5984737458428864,0.9720929426187959,0.5147393442976641,0.9846314718817095,0.4433590238285555,1.0053077452628147,0.3847182074356791,1.0291208313794076,0.3395804960498898,1.0515571362643528,0.30883332915613904,1.0688351569791998,0.29324038791976703,1.0782021523479162};
REAL biquadb[]={1,1.3032179114659426,1,1.331445003936117,1,1.386489638590643,0.9999999999999999,1.4652960679240472,1,1.5627607428081918,1,1.6713344113052224,1,1.7809169395430153,0.9999999999999999,1.8794166876258824,1,1.9542619753540196,1,1.994780595120822};
REAL gain=1/34.49136475541368;
REAL xyv[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int16_t IIR_Calc(int16_t v)
{
	int i,b,xp=0,yp=3,bqp=0;
	REAL out=(float)v*gain;
	for (i=32; i>0; i--) {xyv[i]=xyv[i-1];}
	for (b=0; b<NBQ; b++)
	{
		int len=2;
		xyv[xp]=out;
		for(i=0; i<len; i++) { out+=xyv[xp+len-i]*biquadb[bqp+i]-xyv[yp+len-i]*biquada[bqp+i]; }
		bqp+=len;
		xyv[yp]=out;
		xp=yp; yp+=len+1;
	}
	return (int16_t)out;
}