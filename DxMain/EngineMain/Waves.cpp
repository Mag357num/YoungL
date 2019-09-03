#include "stdafx.h"
#include "Waves.h"

#include <ppl.h>
#include <algorithm>
#include <vector>
#include <assert.h>

using namespace DirectX;

Waves::Waves(int m, int n, float dx, float dt, float speed, float damping)
{
	mNumRows = m;
	mNumColumn = n;

	mVertexCount = m * n;
	mTriangleCount = (m - 1)*(n - 1) * 2;

	mTimeStep = dt;
	mSpatialStep = dx;
	 
	float d = damping * dt + 2.0f;
	float e = (speed*speed)*(dt*dt) / (dx*dx);
	mK1 = (damping*dt - 2.0f) / d;
	mK2 = (4.0f - 8.0f*e) / d;
	mK3 = (2.0f*e) / d;

	mPrevSolution.resize(m*n);
	mCurrSolution.resize(m*n);
	mNormals.resize(m*n);
	mTangentX.resize(m*n);

	//generate grid vertices in system memory.
	float halfWidth = (n - 1)*dx*0.5f;
	float halfDepth = (m - 1)*dx*0.5f;
	for (int i = 0; i < m; i++)
	{
		float z = halfDepth - i * dx;
		for (int j = 0; j < n; j++)
		{
			float x = -halfWidth + j * dx;

			mPrevSolution[i*n + j] = XMFLOAT3(x, 0.0f, z);
			mCurrSolution[i*n + j] = XMFLOAT3(x, 0.0f, z);
			mNormals[i*n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			mTangentX[i*n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);

		}
	}

}

Waves::~Waves()
{

}

int Waves::RowCount()const
{
	return mNumRows;
}

int Waves::ColumnCount()const
{
	return mNumColumn;
}

int Waves::TriangleCount()const
{
	return mTriangleCount;
}

int Waves::VertexCount()const
{
	return mVertexCount;
}

float Waves::Width()const
{
	return mNumColumn * mSpatialStep;
}

float Waves::Depth()const
{
	return mNumRows * mSpatialStep;
}

void Waves::Update(float dt)
{
	static float t = 0;

	//accumulate time
	t += dt;

	//only update the simulation at the specified time sea
	if (t>mTimeStep)
	{
		//only update interior points; we use zero boundary condition
		concurrency::parallel_for(1, mNumRows - 1, [this](int i) {
		//for( i =1; i < nNumRows - 1; ++i)
			for (int j = 1; j<mNumColumn-1;++j)
			{
				//after this update we will be discarding the old previous buffer. so overwrite that buffer with the new update.
				//note how we can do this inplace(read/write to same element)
				//because we won't need prev_ij again and the assignment happens last.

				//note j indexs x and i indexes z:h(x_j, z_i,t_k)
				//moreover, our +z axis goes "down" l this is just to keep consistent with our row indices going down.
				mPrevSolution[i*mNumColumn + j].y =
					mK1 * mPrevSolution[i*mNumColumn + j].y +
					mK2 * mCurrSolution[i*mNumColumn + j].y +
					mK3 * mCurrSolution[(i + 1)*mNumColumn + j].y +
					mCurrSolution[(i - 1)*mNumColumn + j].y +
					mCurrSolution[i*mNumColumn + j + 1].y +
					mCurrSolution[i*mNumColumn + j - 1].y;
			}
		});

		//we just overwrote the previous buffer with the new data. so this data needs to become the current solution and the
		//old current becomes the new previous solution
		std::swap(mPrevSolution, mCurrSolution);

		t = 0.0f;

		//compute normals using finite diffrencec scheme.
		concurrency::parallel_for(1, mNumRows - 1, [this](int i)
		{
			for (int  j = 0; j < mNumColumn; j++)
			{
				float l = mCurrSolution[i*mNumColumn + j - 1].y;
				float r = mCurrSolution[i*mNumColumn + j + 1].y;
				float t = mCurrSolution[(i-1)*mNumColumn + j].y;
				float b = mCurrSolution[(i +1)*mNumColumn + j].y;
				mNormals[i*mNumColumn + j].x = -r + l;
				mNormals[i*mNumColumn + j].y = 2.0f*mSpatialStep;
				mNormals[i*mNumColumn + j].z = b -t;

				XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&mNormals[i*mNumColumn + j]));
				XMStoreFloat3(&mNormals[i*mNumColumn + j], n);

				mTangentX[i*mNumColumn + j] = XMFLOAT3(2.0f*mSpatialStep, r - l, 0.0f);
				XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&mTangentX[i*mNumColumn + j]));
				XMStoreFloat3(&mTangentX[i*mNumColumn + j], T);
			}
		});
	}
}

void Waves::Disturb(int i, int j, float magnitude)
{
	//don't disturb boundaries
	assert(i > 1 && j < mNumRows - 2);
	assert(j > 1 && j < mNumColumn - 2);

	float halfMag = 0.5f*magnitude;
	//disturb the ijth vertex height and its neighbors.
	mCurrSolution[i*mNumColumn + j].y += magnitude;
	mCurrSolution[i*mNumColumn + j + 1].y += halfMag;
	mCurrSolution[i*mNumColumn + j - 1].y += halfMag;
	mCurrSolution[(i + 1)*mNumColumn + j].y += halfMag;
	mCurrSolution[(i - 1)*mNumColumn + j].y += halfMag;
}