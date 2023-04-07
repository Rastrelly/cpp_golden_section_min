#include <iostream>
#include <GL/freeglut.h>
#include <vector>

using std::cout;
using std::cin;
using std::vector;

int wx = 800; int wy = 600;

double ac = 50.0f, bc = -1000.0f, cc = -90.0f;

struct cPoint
{
	double x, y;
};

struct cColor
{
	float r, g, b;
};

struct goldState
{
	double a_init, b_init, a, b, yia, yib, ya, yb;
	cPoint min;
	vector<cPoint> pts;
	vector<cPoint> pts_l;
	vector<cPoint> pts_r;
	float ymax, ymin;
} procRes;

//glut

void cbReshape(int x, int y)
{
	wx = x;
	wy = y;
	glViewport(0, 0, wx, wy);
}

void setupProjection(double baseScaleX, double baseScaleY)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double rwx = 0; double rwy = 0;

	if (wx > wy)
	{
		rwy = 2 * (baseScaleY);
		rwx = 2 * (baseScaleX/2) * ((double)wx / (double)wy);
	}
	else
	{
		rwx = 2 * (baseScaleX/2);
		rwy = 2 * (baseScaleY) * ((double)wy / (double)wx);
	}

	double oLeft = -(rwx / 2);
	double oRight = (rwx / 2);
	double oBottom = -(rwy / 2);
	double oTop = (rwy / 2);

	//printf("L=%f; R=%f; B=%f; T=%f; wx/wy=%f;\n", oLeft, oRight, oBottom, oTop, ((double)wx / (double)wy));

	gluOrtho2D(oLeft, oRight, oBottom, oTop);
}

void DrawPoint(cPoint pt, double rx, double ry, cColor clr)
{
	
	glColor3d(clr.r, clr.g, clr.b);

	glBegin(GL_LINES);

	glVertex2d(pt.x - rx, pt.y - ry);
	glVertex2d(pt.x + rx, pt.y + ry);
	glVertex2d(pt.x + rx, pt.y - ry);
	glVertex2d(pt.x - rx, pt.y + ry);

	glEnd();
}

void DrawChartLine(vector<cPoint> pts, cColor clr)
{
	int l = pts.size();
	for (int i = 0; i < l-1; i++)
	{
		glColor3d(clr.r, clr.g, clr.b);

		glBegin(GL_LINES);

		glVertex2d(pts[i].x, pts[i].y);
		glVertex2d(pts[i+1].x, pts[i+1].y);

		glEnd();
	}
}
void cbDisplay()
{
	
	double bsx = procRes.b_init-procRes.a_init;
	double bsy = procRes.ymax - procRes.ymin;

	setupProjection(1.1*bsx,1.1*bsy);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	double crossScaleX = bsx / 50.0f;
	double crossScaleY = bsy / 50.0f;
	
	glLineWidth(2.0f);
	DrawChartLine(procRes.pts, { 1.0,1.0,1.0 });
	DrawPoint({ procRes.a_init,procRes.yia }, crossScaleX/2, crossScaleY, {1.0,0.0,0.0});
	DrawPoint({ procRes.b_init,procRes.yib }, crossScaleX/2, crossScaleY, { 0.0,1.0,0.0 });

	glLineWidth(1.0f);
	for (int i = 0; i < procRes.pts_l.size(); i++)
	{
		DrawPoint({ procRes.pts_l[i].x,procRes.pts_l[i].y }, crossScaleX/2, crossScaleY, { 0.7,0.0,0.0 });
	}
	for (int i = 0; i < procRes.pts_r.size(); i++)
	{
		DrawPoint({ procRes.pts_r[i].x,procRes.pts_r[i].y }, crossScaleX/2, crossScaleY, { 0.0,0.7,0.0 });
	}

	glLineWidth(2.0f);
	DrawPoint({ procRes.min.x,procRes.min.y }, crossScaleX/2, crossScaleY, { 0.0,0.0,1.0 });

	glutSwapBuffers();
}

void cbIdle()
{
	cbDisplay();
}

//math

double func(double a, double b, double c, double x)
{
	return a * pow(x, 2) + b * x + c;
}

void goldensection(double &a, double &b, double &d, double prop)
{
	double delta = (b - a) / prop;
	double x1 = b - delta;
	double x2 = a + delta;
	double y1 = func(ac, bc, cc, x1);
	double y2 = func(ac, bc, cc, x2);

	if (y1 >= y2) { a = x1; b = b; }
	else { a = a; b = x2; }

	d = abs(b - a);

}

int main(int argc, char **argv)
{
	cout << "Input function coeffs:\n";
	cin >> ac >> bc >> cc;

	double a = 0, b = 0;
	cout << "Input a, b:\n";
	cin >> a >> b;

	if (a > b)
	{
		float buf = b;
		b = a;
		a = buf;
	}
	
	double e = 0.0000001;
	double prop = 1.62;
	double d = b - a;

	procRes.a_init = a;
	procRes.b_init = b;

	procRes.yia = func(ac, bc, cc, a);
	procRes.yib = func(ac, bc, cc, b);

	int attempts = 0;
	while ((d > e) && (attempts<1000000))
	{
		goldensection(a, b, d, prop);

		procRes.pts_l.push_back({ a,func(ac, bc, cc, a) });
		procRes.pts_r.push_back({ b,func(ac, bc, cc, b) });
		printf("%f, %f; %f, %f\n", a, func(ac, bc, cc, a), b, func(ac, bc, cc, b));

		attempts++;
	}

	double xmin = (b + a) / (double)2;
	cout << "Xmin = " << xmin << "\n";

	procRes.a = a;
	procRes.b = b;

	procRes.ya = func(ac, bc, cc, a);
	procRes.yb = func(ac, bc, cc, b);

	procRes.min.x = xmin;
	procRes.min.y = func(ac, bc, cc, xmin);

	double marg = 0.1*(procRes.b_init - procRes.a_init);
	double cx = procRes.a_init - marg;
	double cy = func(ac, bc, cc, cx);
	double dcx = ((procRes.b_init + marg) - (procRes.a_init - marg)) / 1000.0f;

	int i = 0;

	double cymax = cy, cymin = cy;

	while (cx <= procRes.b_init + marg)
	{
		cy = func(ac, bc, cc, cx);
		procRes.pts.push_back({ cx, cy });

		if (cy < cymin) cymin = cy;
		if (cy > cymax) cymax = cy;

		i++;
		cx += dcx;
	}

	procRes.ymin = cymin;
	procRes.ymax = cymax;

	glutInit(&argc, argv); //initializing GLUT

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	glutInitWindowSize(wx, wy);

	glutCreateWindow("GLUT WINDOW");

	//OpenGL setup block
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//GLUT launch block
	glutIdleFunc(cbIdle);
	glutReshapeFunc(cbReshape);
	glutDisplayFunc(cbDisplay);

	glutMainLoop();

	system("pause");
}