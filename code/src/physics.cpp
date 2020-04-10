#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\glm.hpp>
#include <iostream>
#include <glm\gtx\intersect.hpp>

//Exemple
extern void Exemple_GUI();
extern void Exemple_PhysicsInit();
extern void Exemple_PhysicsUpdate(float dt);
extern void Exemple_PhysicsCleanup();
extern bool renderSphere;

bool show_test_window = false;

namespace Sphere {
	glm::vec3 c = glm::vec3(0, 1, 0); //centro esfera
	float r = 1; //radio esfera 

	extern void setupSphere(glm::vec3 pos, float rad);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float rad);
}

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

namespace {
	static struct PhysParams {
		float min = 0.f;
		float max = 10.f;
	} p_pars;


	static struct MeshPoint {
		glm::vec3 newPosition;
		glm::vec3 actualPosition;
		glm::vec3 lastPosition;
		glm::vec3 totalForce; //total
		glm::vec3 structuralForce; //horitzontals i verticals
		glm::vec3 shearForce; //diagonals
		glm::vec3 bendingForce; //contaria?
		float mass = 1;
		
		glm::vec3 velocity;
	

		MeshPoint *rightPoint;
		MeshPoint *leftPoint;
		MeshPoint *upPoint;
		MeshPoint *downPoint;

		int type; //del 1 al 25

	

	};

	static struct ParticleMesh {
		MeshPoint **points;
		glm::vec3 *pointsPositions;
		float kEslatic = 400;
		float kDump = 200;
		float gravity = -9.81;
		float structuralSpringLength = 0.5f;
		float shearSpringLength = glm::sqrt(glm::pow(structuralSpringLength,2)*2);
		float bendingSpringLength = structuralSpringLength * 2;
		float dtReductor = 0.1f;
		float E = 0.05f;
		float automaticTimeReset = 20;
		float timeCount;

	} myPM;
}
void resetMyPM() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.points[i][j].actualPosition.x = i * 0.5f - 17 * 0.5f * 0.5f; //posicio inicial = horitzontal i elevat
			myPM.points[i][j].actualPosition.y = 9;
			myPM.points[i][j].actualPosition.z = j * 0.5f - 13 * 0.5f * 0.5f;
			myPM.points[i][j].lastPosition = myPM.points[i][j].actualPosition;
			myPM.points[i][j].velocity = glm::vec3(0, 0, 0); //velocitat inicial = 0
		}
	}
	myPM.timeCount = 0;
	Sphere::c = glm::vec3((rand() % 11) - 5, (rand() % 5) + 1 , (rand() % 11) - 5);
	Sphere::r = rand() % 2 + 1;
}

void updateMyPMPointsPositions() {
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.pointsPositions[i * 14 + j] = glm::vec3(myPM.points[i][j].actualPosition.x, myPM.points[i][j].actualPosition.y, myPM.points[i][j].actualPosition.z);
		}
	}
}

glm::vec3 getSpringForce(MeshPoint P1, MeshPoint P2 , float L12) {

	glm::vec3 auxVec3 = (P1.actualPosition - P2.actualPosition) / glm::length<float>(P1.actualPosition - P2.actualPosition);
	float firstComponent = myPM.kEslatic * (glm::length<float>(P1.actualPosition - P2.actualPosition) - L12);
	float secondComponent = myPM.kDump * (glm::dot<float>(auxVec3,P1.velocity-P2.velocity));
	
	return -(firstComponent + secondComponent) * auxVec3;
}
////////////////////

namespace myData {
	float indiceRebote = 2;

	//planos
	glm::vec3 XYn = glm::vec3(0, 0, 1);//normal de plano XY es Z
	glm::vec3 YZn = glm::vec3(1, 0, 0);
	glm::vec3 XZn = glm::vec3(0, 1, 0);
	glm::vec3 negYZn = glm::vec3(-1, 0, 0);
	glm::vec3 negXYn = glm::vec3(0, 0, -1);
	glm::vec3 negXZn = glm::vec3(0, -1, 0);

	glm::vec3 aux = glm::vec3(-5, 0, -5);
	glm::vec3 aux2 = glm::vec3(-5, 0, 5);
	glm::vec3 aux3 = glm::vec3(5, 0, -5);
	glm::vec3 aux4 = glm::vec3(-5, 10, -5);
	glm::vec3 vX1 = aux - aux2;
	glm::vec3 vX2 = aux - aux3;

	float planeD(glm::vec3 normal, glm::vec3 point) {
		return -(normal.x*point.x + normal.y*point.y + normal.z*point.z);
	}

}



void MyPhysicsInit() {

	ClothMesh::setupClothMesh();

	myPM.timeCount = 0;
	renderSphere = true;
	myPM.points = new MeshPoint*[18];
	for (int i = 0; i < 18; i++) {
		myPM.points[i] = new MeshPoint[14];
	}
	myPM.pointsPositions = new glm::vec3[14*18];
	std::cout << "Mapa de Tipos: " << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {
			myPM.points[i][j].actualPosition.x = i * 0.5f - 17 * 0.5f * 0.5f; //posicio inicial = horitzontal i elevat
			myPM.points[i][j].actualPosition.y = 9;
			myPM.points[i][j].actualPosition.z = j * 0.5f - 13 * 0.5f * 0.5f;
			myPM.points[i][j].lastPosition = myPM.points[i][j].actualPosition;

			myPM.points[i][j].velocity = glm::vec3(0,0,0); //velocitat inicial = 0
			myPM.points[i][j].type = 0;


			//types i pointers
			//pointers laterals
			if (i == 0) {
				myPM.points[i][j].leftPoint = nullptr;
				myPM.points[i][j].rightPoint = &myPM.points[i + 1][j];
				if (j > 1 && j < 12) {
					myPM.points[i][j].type = 3;
				}
				else if (j == 1) {
					myPM.points[i][j].type = 15;
				}
				else if (j == 12) {
					myPM.points[i][j].type = 14;
				}
				else if (j == 0) {
					myPM.points[i][j].type = 2;
				}
				else if (j == 13) {
					myPM.points[i][j].type = 1;
				}
			}
			else if (i == 17) {
				myPM.points[i][j].leftPoint = &myPM.points[i - 1][j];
				myPM.points[i][j].rightPoint = nullptr;
				if (j > 1 && j < 12) {
					myPM.points[i][j].type = 6;
				}
				else if (j == 1) {
					myPM.points[i][j].type = 17;
				}
				else if (j == 12) {
					myPM.points[i][j].type = 16;
				}
				else if (j == 0) {
					myPM.points[i][j].type = 5;
				}
				else if (j == 13) {
					myPM.points[i][j].type = 4;
				}
			}
			else {
				myPM.points[i][j].leftPoint = &myPM.points[i-1][j];
				myPM.points[i][j].rightPoint = &myPM.points[i+1][j];
				if (i == 1) {
					if (j > 1 && j < 12) {
						myPM.points[i][j].type = 22;
					}
					else if (j == 1) {
						myPM.points[i][j].type = 20;
					}
					else if (j == 12) {
						myPM.points[i][j].type = 18;
					}
					else if (j == 0) {
						myPM.points[i][j].type = 12;
					}
					else if (j == 13) {
						myPM.points[i][j].type = 10;
					}
				}
				else if (i == 16) {
					if (j > 1 && j < 12) {
						myPM.points[i][j].type = 23;
					}
					else if (j == 1) {
						myPM.points[i][j].type = 21;
					}
					else if (j == 12) {
						myPM.points[i][j].type = 19;
					}
					else if (j == 0) {
						myPM.points[i][j].type = 13;
					}
					else if (j == 13) {
						myPM.points[i][j].type = 11;
					}
				}
				else {
					if (j > 1 && j < 12) {
						myPM.points[i][j].type = 9;
					}
					else if (j == 1) {
						myPM.points[i][j].type = 25;
					}
					else if (j == 12) {
						myPM.points[i][j].type = 24;
					}
					else if (j == 0) {
						myPM.points[i][j].type = 8;
					}
					else if (j == 13) {
						myPM.points[i][j].type = 7;
					}
				}
			}

			//pointers verticals
			if (j == 0) {
				myPM.points[i][j].downPoint = nullptr;
				myPM.points[i][j].upPoint = &myPM.points[i][j+1];
			}
			else if (j == 13) {
				myPM.points[i][j].downPoint = &myPM.points[i][j-1];
				myPM.points[i][j].upPoint = nullptr;
			}
			else {
				myPM.points[i][j].downPoint = &myPM.points[i][j-1];
				myPM.points[i][j].upPoint = &myPM.points[i][j+1];
			}
			if (myPM.points[i][j].type < 10)
				std::cout << " ";
		    std::cout << myPM.points[i][j].type << " ";

		}
		std::cout << std::endl;
	}

	std::cout << "-----------------------------------------" << std::endl;
	updateMyPMPointsPositions();
	ClothMesh::updateClothMesh(&(myPM.pointsPositions[0].x));


}

void MyPhysicsUpdate(float dt) {

	if (myPM.timeCount >= myPM.automaticTimeReset) {
		resetMyPM();
	}
	else {
		myPM.timeCount += dt;
	}

	//forçes
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {

			myPM.points[i][j].structuralForce = glm::vec3(0,0,0);
			myPM.points[i][j].shearForce = glm::vec3(0, 0, 0);
			myPM.points[i][j].bendingForce = glm::vec3(0, 0, 0);
			myPM.points[i][j].totalForce = glm::vec3(0, 0, 0);


			

			switch (myPM.points[i][j].type) {
				case 1:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->rightPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 2:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->rightPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 3:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->rightPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->rightPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 4:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->leftPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 5:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->leftPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 6:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->leftPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->leftPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 7:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 8:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 9:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				case 10:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 11:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 12:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 13:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 14:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->rightPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->rightPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 15:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->rightPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->rightPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 16:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->leftPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->leftPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 17:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->leftPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->leftPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


				case 18:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 19:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 20:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 21:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 22:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 23:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					break;
				case 24:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint->downPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				case 25:
					//structural
					myPM.points[i][j].structuralForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint, myPM.structuralSpringLength);
					myPM.points[i][j].structuralForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].downPoint, myPM.structuralSpringLength);
					//shear
					myPM.points[i][j].shearForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->upPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->downPoint, myPM.shearSpringLength);
					myPM.points[i][j].shearForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->upPoint, myPM.shearSpringLength);
					//bending
					myPM.points[i][j].bendingForce = getSpringForce(myPM.points[i][j], *myPM.points[i][j].upPoint->upPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].leftPoint->leftPoint, myPM.bendingSpringLength);
					myPM.points[i][j].bendingForce += getSpringForce(myPM.points[i][j], *myPM.points[i][j].rightPoint->rightPoint, myPM.bendingSpringLength);
					break;
				default:
					break;
					

			}
				
			myPM.points[i][j].totalForce = myPM.points[i][j].structuralForce + myPM.points[i][j].shearForce + myPM.points[i][j].bendingForce + glm::vec3(0, myPM.gravity, 0);
			
		}
	}


	//actualitza posicions i velocitat apart amb Verlet
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {

			if (!((i == 0 && j == 0) || (i == 0 && j == 13))) {
				myPM.points[i][j].newPosition = myPM.points[i][j].actualPosition + (myPM.points[i][j].actualPosition - myPM.points[i][j].lastPosition) + (myPM.points[i][j].totalForce / myPM.points[i][j].mass)*glm::pow(dt*myPM.dtReductor, 2);


				//Colisions

				//plano tierra                                                                                                                                       
				if (((glm::dot(myData::XZn, myPM.points[i][j].actualPosition) + myData::planeD(myData::XZn, myData::aux))*(glm::dot(myData::XZn, myPM.points[i][j].newPosition) + myData::planeD(myData::XZn, myData::aux))) <= 0) {

					myPM.points[i][j].newPosition = myPM.points[i][j].newPosition - (1 + myPM.E) * (glm::dot(myData::XZn, myPM.points[i][j].newPosition) + myData::planeD(myData::XZn, myData::aux))*myData::XZn;

				}

				//plano derecha
				if (((glm::dot(myData::negYZn, myPM.points[i][j].actualPosition) + myData::planeD(myData::negYZn, myData::aux3))*(glm::dot(myData::negYZn, myPM.points[i][j].newPosition) + myData::planeD(myData::negYZn, myData::aux3))) <= 0) {
					myPM.points[i][j].newPosition = myPM.points[i][j].newPosition - (1 + myPM.E) * (glm::dot(myData::negYZn, myPM.points[i][j].newPosition) + myData::planeD(myData::negYZn, myData::aux3))*myData::negYZn;

				}

				//plano delante
				if (((glm::dot(myData::negXYn, myPM.points[i][j].actualPosition) + myData::planeD(myData::negXYn, myData::aux3))*(glm::dot(myData::negXYn, myPM.points[i][j].newPosition) + myData::planeD(myData::negXYn, myData::aux3))) <= 0) {
					myPM.points[i][j].newPosition = myPM.points[i][j].newPosition - (1 + myPM.E) * (glm::dot(myData::negXYn, myPM.points[i][j].newPosition) + myData::planeD(myData::negXYn, myData::aux3))*myData::negXYn;

				}

				//plano detras
				if (((glm::dot(myData::XYn, myPM.points[i][j].actualPosition) + myData::planeD(myData::XYn, myData::aux2))*(glm::dot(myData::XYn, myPM.points[i][j].newPosition) + myData::planeD(myData::XYn, myData::aux2))) <= 0) {
					myPM.points[i][j].newPosition = myPM.points[i][j].newPosition - (1 + myPM.E) * (glm::dot(myData::XYn, myPM.points[i][j].newPosition) + myData::planeD(myData::XYn, myData::aux2))*myData::XYn;

				}

				//plano izquierda
				if (((glm::dot(myData::YZn, myPM.points[i][j].actualPosition) + myData::planeD(myData::YZn, myData::aux2))*(glm::dot(myData::YZn, myPM.points[i][j].newPosition) + myData::planeD(myData::YZn, myData::aux2))) <= 0) {
					myPM.points[i][j].newPosition = myPM.points[i][j].newPosition - (1 + myPM.E) * (glm::dot(myData::YZn, myPM.points[i][j].newPosition) + myData::planeD(myData::YZn, myData::aux2))*myData::YZn;

				}

				//plano arriba
				if (((glm::dot(myData::negXZn, myPM.points[i][j].actualPosition) + myData::planeD(myData::negXZn, myData::aux4))*(glm::dot(myData::negXZn, myPM.points[i][j].newPosition) + myData::planeD(myData::negXZn, myData::aux4))) <= 0) {
					myPM.points[i][j].newPosition = myPM.points[i][j].newPosition - (1 + myPM.E) * (glm::dot(myData::negXZn, myPM.points[i][j].newPosition) + myData::planeD(myData::negXZn, myData::aux4))*myData::negXZn;

				}

				//renderSphere = true;
				//Colisiones esfera
				if (renderSphere) {
					Sphere::updateSphere(Sphere::c, Sphere::r);
					glm::vec3 nPlane = (myPM.points[i][j].actualPosition - Sphere::c);

					if (glm::sqrt(glm::pow(nPlane.x, 2) + glm::pow(nPlane.y, 2) + glm::pow(nPlane.z, 2)) <= Sphere::r) {
						glm::vec3 punto;
						glm::vec3 normal;
						glm::intersectLineSphere(myPM.points[i][j].newPosition, myPM.points[i][j].actualPosition, Sphere::c, Sphere::r, punto, normal);
						float D = -(normal.x*punto.x + normal.y*punto.y + normal.z*punto.z);

						myPM.points[i][j].newPosition = myPM.points[i][j].newPosition - (1 + myPM.E) * (glm::dot(normal, myPM.points[i][j].newPosition) + D)*normal;

					}

				}

				//Actualitzar posició
				myPM.points[i][j].lastPosition = myPM.points[i][j].actualPosition;

				myPM.points[i][j].velocity = (myPM.points[i][j].newPosition - myPM.points[i][j].actualPosition) / dt*myPM.dtReductor;

				myPM.points[i][j].actualPosition = myPM.points[i][j].newPosition;
			}
		}
	}
	updateMyPMPointsPositions();
	ClothMesh::updateClothMesh(&(myPM.pointsPositions[0].x));
	
}

void MyPhysicsCleanup() {
	ClothMesh::cleanupClothMesh();

	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 14; j++) {

			delete myPM.points[i][j].downPoint;
			myPM.points[i][j].downPoint = nullptr;

			delete myPM.points[i][j].upPoint;
			myPM.points[i][j].upPoint = nullptr;

			delete myPM.points[i][j].leftPoint;
			myPM.points[i][j].leftPoint = nullptr;
			
			delete myPM.points[i][j].rightPoint;
			myPM.points[i][j].rightPoint = nullptr;
		}
	}

	for (int i = 0; i < 18; i++) delete[] myPM.points[i];
	delete[] myPM.points;
	delete[] myPM.pointsPositions;


}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);
	ImGui::SetWindowSize(ImVec2(400, 300));
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		//Exemple_GUI();
		ImGui::Checkbox("Esfera", &renderSphere);
		ImGui::SliderFloat("Fuerza rebote", &myPM.E, 0.f,1.f);
		ImGui::DragFloat("Gravedad", &myPM.gravity, 0.01f);
		ImGui::DragFloat("Reductor dt", &myPM.dtReductor, 0.01f);
		ImGui::SliderFloat("Tiempo de reseteo automatico", &myPM.automaticTimeReset, 10.f, 30.f);
		if (ImGui::Button("Reseteo manual"))
		{
			resetMyPM();
		}
	}

	ImGui::End();
}

/////////////////////////////////

void PhysicsInit() {
	MyPhysicsInit();
}

void PhysicsUpdate(float dt) {
	MyPhysicsUpdate(dt);
}

void PhysicsCleanup() {
	MyPhysicsCleanup();
}