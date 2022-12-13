
#define STACK_SIZE 50

#define MAT_SIZE 4

/* can be changed for row first storage */
#define access(row, col) ((row)*MAT_SIZE + (col))

typedef float mtype;

void PrintMatrix(char *str, mtype *mat);
void Normalize(mtype *vec);
void CrossProduct(mtype *v1, mtype *v2, mtype *res);

void MakeUnitMatrix(mtype *mat);
void InitializeMatrixStack(void);
int PushMatrix(void);
int PopMatrix(void);
void MultMatrices(mtype *src1, mtype *src2, mtype *res);
void MultMatrix(mtype *mat);
void Scale(mtype *vals);
void Translate(mtype *vals);
void Rotate(mtype *vals);
void Rotate4(mtype angle, mtype x, mtype y, mtype z);

void GetMatrix(mtype *mat);
void TransformPoint(mtype *src, mtype *mat, mtype *dest);
void Transform3Point(mtype *src, mtype *mat, mtype *dest);
void Transform3Vector(mtype *src, mtype *mat, mtype *dest);
void InverseMatrix(mtype *srcmat, mtype *destmat);
