/* Make a background file for cpfg, 
from the obstacle file used by the environment program (obcheck) 
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

void define_material (FILE *fp_out);

int main(int argc, char *argv[])
{
	float length, width, xa, xb, ya,yb, dx, dy, sina, cosa;
	FILE *fp_in, *fp_out;

	if (argc !=3) {
		fprintf(stderr, "Usage: %s obstacles.dat background.dat\n", argv[0]);
		exit(1);
	};

	if((fp_in = fopen(argv[1],"r")) == NULL) {
		fprintf(stderr, "%s: Can't open %s\n", argv[0], argv[1]);
		exit(1);
	};

	if((fp_out = fopen(argv[2],"w")) == NULL) {
		fprintf(stderr, "%s: Can't open %s\n", argv[0], argv[2]);
		exit(1);
	};

	define_material(fp_out);

	fscanf(fp_in, "line width: %f\n", &width);

	fscanf(fp_in, "coordinates:\n");
	while (fscanf(fp_in, "%f %f %f %f", &xa,&ya,&xb,&yb) == 4) {
		dx = xb-xa;
		dy = yb-ya;
		length = sqrt(dx*dx+dy*dy);
		cosa = dy/length;
		sina = dx/length;
		fprintf(fp_out, "pushmatrix\n");
		fprintf(fp_out, "multmatrix\n");
		fprintf(fp_out, "%f %f %f %f\n", cosa, -sina, 0.0, 0.0);
		fprintf(fp_out, "%f %f %f %f\n", sina, cosa, 0.0, 0.0);
		fprintf(fp_out, "%f %f %f %f\n", 0.0, 0.0, 1.0, 0.0);
		fprintf(fp_out, "%f %f %f %f\n", xa, ya, 0.0, 1.0);
		fprintf(fp_out, "cylinder %f %f\n", width, length);
		fprintf(fp_out, "popmatrix\n");
	}
	fclose(fp_in);
	fclose(fp_out);

        return(0);
}

void define_material(fp_out)
FILE *fp_out;
{
	fprintf(fp_out, "material\n");
	fprintf(fp_out, "0.6 0.2 0 1\n"); /* ambient */
	fprintf(fp_out, "1.0 0.25 0 1\n"); /* diffuse */
	fprintf(fp_out, "1.0 0.25 0.5 1\n"); /* specular */
	fprintf(fp_out, "0 0 0 1\n"); /* emmisive */
	fprintf(fp_out, "5\n\n"); /* specular exponent */
       
        return;
}
