#include "main.h"

#define width	1280
#define height	720

int thisone;
int dpi = 72;
int n = width*height;
int aadepth = 5;
RGBType *pixels = new RGBType[n];
RGBType pix[width][height];
double aathreshold = 0.1;
double aspectratio = (double) width / (double) height;
double ambientlight = 0.2;
double accuracy = 0.00000001;

Vect O(0, 0, 0);
Vect X(1, 0, 0);
Vect Y(0, 1, 0);
Vect Z(0, 0, 1);

Vect new_sphere_location(1.75, -0.25, 0);

Vect campos(0, 3.0, -6);

Vect look_at(0, 0, 0);
Vect diff_btw(campos.getVectX() - look_at.getVectX(), campos.getVectY() - look_at.getVectY(), campos.getVectZ() - look_at.getVectZ());

Vect camdir = diff_btw.negative().normalize();
Vect camright = Y.crossProduct(camdir).normalize();
Vect camdown = camright.crossProduct(camdir);
Camera scene_cam(campos, camdir, camright, camdown);

Color white_light(1.0, 1.0, 1.0, 0);
Color pretty_green(0.5, 1.0, 0.5, 0.3);
Color maroon(1, .2, 0, 0.1);
Color tile_floor(1, 1, 1, 2);
Color gray(0.5, 0.5, 0.5, 0);
Color black(0.0, 0.0, 0.0, 0);

Vect light_position(-7, 10, -10);

Light scene_light1(Vect(-7, 10, -10), white_light);
Light scene_light2(Vect(7, 10, 10), white_light);

vector<Source*> light_sources;

// scene objects
Sphere scene_sphere1(Vect(0, 0, 0), 1, Color(.9, .9, .9, 0.0));
Sphere scene_sphere2(Vect(2.25, 0, 0), 1, Color(.3, .3, .3, 0.0));
Sphere scene_sphere3(Vect(-2.25, 0, 0), 1, Color(0.0, 0, .5, 0.3));
Sphere scene_sphere4(Vect(-1.0, 0, 1.5), 1, Color(0.5, 0, .8, 0.0));
Sphere scene_sphere5(Vect(1.0, 0, 1.5), 1, Color(0.0, 0.5, .5, 0.8));
Sphere scene_sphere6(Vect(0, 0, 4), 1, Color(0.5, 0.1, .2, 0.2));

Plane scene_plane(Y, -1, tile_floor);

vector<Object*> scene_objects;


int aa_index;
double xamnt, yamnt;
double tempRed, tempGreen, tempBlue;

void renderTask(int i, int j, int xlength, int ylength){

	int _i = 0, total = height*width*aadepth*aadepth; 
	double percent = (double)(100 * _i) / (double)total;
	for (int x = i; x < xlength; x++) {
		for (int y = j; y < ylength; y++) {
			thisone = y*xlength + x;

			// start with a blank pixel
			double *tempRed = new double[aadepth*aadepth];
			double *tempGreen = new double[aadepth*aadepth];
			double *tempBlue = new double[aadepth*aadepth];

			for (int aax = 0; aax < aadepth; aax++) {
				for (int aay = 0; aay < aadepth; aay++) {
					if (percent >= 100)
					percent = 100.00;
					else
					percent = ((double)(100 *_i) / (double)total);

					_i++;

					percent = (double) ((int) (percent * 100)) / 100;

					printf_s("\r Progress: %4g%%", percent);
					
					aa_index = aay*aadepth + aax;

					srand(time(0));

					// create the ray from the camera to this pixel
					if (aadepth == 1) {

						// start with no anti-aliasing
						if (width > height) {
							// the image is wider than it is tall
							xamnt = ((x + 0.5) / width)*aspectratio - (((width - height) / (double) height) / 2);
							yamnt = ((height - y) + 0.5) / height;
						}
						else if (height > width) {
							// the imager is taller than it is wide
							xamnt = (x + 0.5) / width;
							yamnt = (((height - y) + 0.5) / height) / aspectratio - (((height - width) / (double) width) / 2);
						}
						else {
							// the image is square
							xamnt = (x + 0.5) / width;
							yamnt = ((height - y) + 0.5) / height;
						}
					}
					else {
						// anti-aliasing
						if (width > height) {
							// the image is wider than it is tall
							xamnt = ((x + (double) aax / ((double) aadepth - 1)) / width)*aspectratio - (((width - height) / (double) height) / 2);
							yamnt = ((height - y) + (double) aax / ((double) aadepth - 1)) / height;
						}
						else if (height > width) {
							// the imager is taller than it is wide
							xamnt = (x + (double) aax / ((double) aadepth - 1)) / width;
							yamnt = (((height - y) + (double) aax / ((double) aadepth - 1)) / height) / aspectratio - (((height - width) / (double) width) / 2);
						}
						else {
							// the image is square
							xamnt = (x + (double) aax / ((double) aadepth - 1)) / width;
							yamnt = ((height - y) + (double) aax / ((double) aadepth - 1)) / height;
						}
					}

					Vect cam_ray_origin = scene_cam.getCameraPosition();
					Vect cam_ray_direction = camdir.vectAdd(camright.vectMult(xamnt - 0.5).vectAdd(camdown.vectMult(yamnt - 0.5))).normalize();

					Ray cam_ray(cam_ray_origin, cam_ray_direction);

					vector<double> intersections;

					for (int index = 0; index < scene_objects.size(); index++) {
						intersections.push_back(scene_objects.at(index)->findIntersection(cam_ray));
					}

					int index_of_winning_object = winningObjectIndex(intersections);

					if (index_of_winning_object == -1) {
						// set the backgroung black
						tempRed[aa_index] = 0;
						tempGreen[aa_index] = 0;
						tempBlue[aa_index] = 0;
					}
					else{
						// index coresponds to an object in our scene
						if (intersections.at(index_of_winning_object) > accuracy) {
							// determine the position and direction vectors at the point of intersection

							Vect intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersections.at(index_of_winning_object)));
							Vect intersecting_ray_direction = cam_ray_direction;

							Color intersection_color = getColorAt(intersection_position, intersecting_ray_direction, scene_objects, index_of_winning_object, light_sources, accuracy, ambientlight);

							tempRed[aa_index] = intersection_color.getColorRed();
							tempGreen[aa_index] = intersection_color.getColorGreen();
							tempBlue[aa_index] = intersection_color.getColorBlue();
						}
					}
				}
			}

			// average the pixel color
			double totalRed = 0;
			double totalGreen = 0;
			double totalBlue = 0;

			for (int iRed = 0; iRed < aadepth*aadepth; iRed++) {
				totalRed = totalRed + tempRed[iRed];
			}
			for (int iGreen = 0; iGreen < aadepth*aadepth; iGreen++) {
				totalGreen = totalGreen + tempGreen[iGreen];
			}
			for (int iBlue = 0; iBlue < aadepth*aadepth; iBlue++) {
				totalBlue = totalBlue + tempBlue[iBlue];
			}

			double avgRed = totalRed / (aadepth*aadepth);
			double avgGreen = totalGreen / (aadepth*aadepth);
			double avgBlue = totalBlue / (aadepth*aadepth);

			pix[x][y].r = avgRed;
			pix[x][y].g = avgGreen;
			pix[x][y].b = avgBlue;

		}
	}
}


int render() {
	printf_s(" Rendering...\n");

	clock_t t1, t2;
	t1 = clock();


	light_sources.push_back(dynamic_cast<Source*>(&scene_light1));
	light_sources.push_back(dynamic_cast<Source*>(&scene_light2));

	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere1));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere2));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere3));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere4));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere5));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere6));

	scene_objects.push_back(dynamic_cast<Object*>(&scene_plane));
	/*
	thread thread1(renderTask, 0, 0, width / 2, height / 2);
	thread thread2(renderTask, 0, height / 2, width / 2, height);
	thread thread3(renderTask, width / 2, 0, width, height / 2);
	thread thread4(renderTask, width / 2, height / 2, width, height);
	*/

	thread t(renderTask, 0, 0, width, height);
	t.join();
	/*
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	*/
	

	for (int i = 0; i < width; i++){
		for (int j = 0; j < height; j++){
			int thisPixel = j*width + i;
			pixels[thisPixel] = pix[i][j];
		}
	}

	savebmp("scene_aa.bmp", width, height, dpi, pixels);

	delete pixels, tempRed, tempGreen, tempBlue;

	t2 = clock();
	float diff = ((float) t2 - (float) t1) / 1000;

	printf_s("\n Total: %g", diff);
	//getchar();
	return 0;
}

void myinit(void) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(1.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
	glMatrixMode(GL_MODELVIEW);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(4);
	glBegin(GL_POINTS);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {

			glColor3d(pix[x][y].r, pix[x][y].g, pix[x][y].b);
			glVertex2i(x, y);
		}

	}
	glEnd();
	glFlush();
}

int main(int argc, char **argv) {
	cout << " Antialiasing depth : "; cin >> aadepth;
	
	if (!(aadepth > 0 && aadepth < 64)){
		printf_s("Antialiasing %d eronat!. Exit...", aadepth);
		return 0;
	}
	render();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Ray Tracer");
	glutDisplayFunc(display);
	myinit();
	glutMainLoop();
	return 0;
}