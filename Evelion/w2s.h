#pragma once


float gWorldToScreen[16] = {0};

int width = 800;   // Default safe values to prevent divide-by-zero
int height = 600;

bool ScreenTransform(Vector3 vPoint, float* vScreen)
{
	if (!vScreen) return false;
	
	vScreen[0] = gWorldToScreen[0] * vPoint.x + gWorldToScreen[4] * vPoint.y + gWorldToScreen[8] * vPoint.z + gWorldToScreen[12];
	vScreen[1] = gWorldToScreen[1] * vPoint.x + gWorldToScreen[5] * vPoint.y + gWorldToScreen[9] * vPoint.z + gWorldToScreen[13];
	float z = gWorldToScreen[3] * vPoint.x + gWorldToScreen[7] * vPoint.y + gWorldToScreen[11] * vPoint.z + gWorldToScreen[15];

	if (z == 0.0f)
	{
		vScreen[0] = 0.0f;
		vScreen[1] = 0.0f;
		return false;
	}
	else
	{
		float invZ = 1.0f / z;
		vScreen[0] = vScreen[0] * invZ;
		vScreen[1] = vScreen[1] * invZ;
		return 0.0f >= invZ;
	}
}


bool WorldToScreen(Vector3 vPoint, float* vScreen)
{
	if (!vScreen || width <= 0 || height <= 0) return false;
	
	bool iResult = ScreenTransform(vPoint, vScreen);
	if (vScreen[0] < 1 && vScreen[1] < 1 && vScreen[0] > -1 && vScreen[1] > -1 && !iResult)
	{
		vScreen[0] = vScreen[0] * (width / 2) + (width / 2);
		vScreen[1] = -vScreen[1] * (height / 2) + (height / 2);
		return true;
	}
	return false;
}