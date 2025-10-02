#define PI 3.141592f

void R(float alpha, float* r, float* v, int a){
	float M[3][3];
	float tmp[3]={0,0,0};
	float c=cos(alpha), s=sin(alpha), t=1-c, x=r[0], y=r[1], z=r[2];
	int i,j;

	M[0][0] = t*x*x + c;
	M[0][1] = t*x*y - z*s;
	M[0][2] = t*x*z + y*s;
	M[1][0] = t*x*y + z*s;
	M[1][1] = t*y*y + c;
	M[1][2] = t*y*z - x*s;
	M[2][0] = t*x*z - y*s;
	M[2][1] = t*y*z + x*s;
	M[2][2] = t*z*z + c;

	for (i=0;i<3;i++){
		for(j=0; j<3; j++){
			tmp[i]+=M[i][j]*v[j+a*3];
		}
		//printf("matrix : %7f %7f %7f \n", M[i][0], M[i][1], M[i][2]);
	}
	v[0+a*3]=tmp[0];
	v[1+a*3]=tmp[1];
	v[2+a*3]=tmp[2];
}

void cross(float *a, float *b, float *c){
	c[0]=a[1]*b[2] - a[2]*b[1];		c[1]=a[2]*b[0] - a[0]*b[2];		c[2]=a[0]*b[1] - a[1]*b[0];
}

void Normalize(float* v)
{
	float div = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	
	if(div!=0.0f)
	{
		v[0] /= div;
		v[1] /= div;
		v[2] /= div;
	}
}

int Max(int a, int b)
{
	return a > b ? a : b;
}

int Min(int a, int b)
{
	return a < b ? a : b;
}

float Min(float a, float b)
{
	return a < b ? a : b;
}


float Abs(float a)
{
	if(a<0.0f) return -a;
	return a;
}

float calcNextRadius(float radius, float area)
{
	float r;

	return r = (-2.0f*PI*radius + sqrt(2.0f*PI*area)) / (2.0f*PI);
}

//rank starts from 0
float punishTheWeak(int rank, int elements, int lastBest, int firstWeakest, float lastBestMeasure, float firstWeakMeasure)
{
	float result;

	if(rank <= lastBest)
	{
		if(lastBest == 0) return 1.0f;

		result = (float)(rank - lastBest) / (float)(-1.0f*lastBest);
		return result = result * (1.0f - lastBestMeasure) + lastBestMeasure;
	}
	else if(lastBest < rank && rank <= firstWeakest)
	{
		if(lastBest == 0) return 1.0f;

		result = (float)(rank - firstWeakest) / (float)(lastBest - firstWeakest);
		return result = result * (lastBestMeasure - firstWeakMeasure) + firstWeakMeasure;
	}
	else if(rank > firstWeakest)
	{
		if(lastBest == 0) return 1.0f;

		result = ((float)rank - elements - 1) / (float)(firstWeakest - elements - 1);
		return result = result * firstWeakMeasure;
	}
	else return -1.0f;
	
}

float distributeEnergyGeom(int rank, float median, int elements, float start)
{
	float r, target = (float)elements * median;

	//return 1.0f;
	//if(rank <= (int)target) return 2.0f;
	//else return 0.5f;
	/*if(rank == 0) return start;
	else if(rank == 1) return start;
	else return 0.5f;*/
	if(target <=1.1f && !rank) return start;
	else if(target <=1.1f && rank) return 0.5f;

	r = ((float)rank - (target - 1.0f))/ ((target - 1.0f)/ -start);

	if (r < 0.5f) return 0.5f;

	return r;
}
