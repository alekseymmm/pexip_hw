#ifndef GEOMETRY_H_
#define GEOMETRY_H_

struct point {
	long x;
	long y;
};

void rotate_point(struct point *pt, int width, int height, struct point *res,
		   float a);

static inline long sign(struct point *p1, struct point *p2, struct point *p3)
{
	return (p1->x - p3->x) * (p2->y - p3->y) -
	       (p2->x - p3->x) * (p1->y - p3->y);
}

int point_in_triangle(struct point *pt, struct point *v1, struct point *v2,
		      struct point *v3);

int point_in_triangle2(struct point *pt, struct point *v1, struct point *v2,
		      struct point *v3);

#endif /* GEOMETRY_H_ */
