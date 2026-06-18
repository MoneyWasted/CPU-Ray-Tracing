// RayTracer.hlsl - GPU Ray Tracing Compute Shader

cbuffer RayTraceConstants : register(b0)
{
	uint gWidth;
	uint gHeight;
	uint gSamples;
	uint gGISamples;
	uint gFrameIndex;
	uint gMaxDepth;
	float gAmbientOcclusionIntensity;
	int gUseTextures;
	int gSoftShadows;
	int gAmbientOcclusion;
	uint gNumSpheres;
	uint gNumQuads;
	uint gNumLights;
	uint gNumTextures;
	float3 gCameraPosition;
	float _pad1;
	float4x4 gCameraRayMatrix;
};

struct Sphere
{
	float3 Position;
	float Radius;
	float3 Color;
	float Reflection;
	float Refraction;
	float Eta;
	float ODRadius;
	int TextureIndex;
};

struct Quad
{
	float3 a;
	float Reflection;
	float3 b;
	float Refraction;
	float3 c;
	float Eta;
	float3 d;
	float ODEta;
	float3 Color;
	int TextureIndex;
	float3 Normal;
	float D;
	float3 Tangent;
	float D1;
	float3 Binormal;
	float D2;
	float3 m;
	float D3;
	float3 Offset;
	float D4;
};

struct Light
{
	float3 Position;
	float Radius;
	float3 Color;
	int IsSphere;
	int ObjectIndex;
	float Ambient;
	float Diffuse;
	float _pad;
};

struct TextureInfo
{
	uint Width;
	uint Height;
	uint _pad0;
	uint _pad1;
};

StructuredBuffer<Sphere> gSpheres : register(t0);
StructuredBuffer<Quad> gQuads : register(t1);
StructuredBuffer<Light> gLights : register(t2);
Texture2D gSceneTextures[16] : register(t3);
StructuredBuffer<TextureInfo> gTextureInfo : register(t19);
RWTexture2D<float4> gOutput : register(u0);
SamplerState gSampler : register(s0);

// Utility functions
float3 normalize_safe(float3 v)
{
	return v / sqrt(dot(v, v) + 0.0001f);
}

float length2(float3 v)
{
	return dot(v, v);
}

float3 reflect_vec(float3 I, float3 N)
{
	return I - 2.0f * dot(I, N) * N;
}

float3 refract_vec(float3 I, float3 N, float eta)
{
	float cosi = -dot(I, N);
	float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
	if (k < 0.0f) return float3(0, 0, 0);
	return eta * I + (eta * cosi - sqrt(k)) * N;
}

float3 mix_vec(float3 a, float3 b, float t)
{
	return a * (1.0f - t) + b * t;
}

float clamp01(float v)
{
	return max(0.0f, min(1.0f, v));
}

float3 SampleSceneTexture(int textureIndex, float2 uv, float3 fallbackColor)
{
	if (gUseTextures == 0 || textureIndex < 0 || textureIndex >= (int)gNumTextures)
	{
		return fallbackColor;
	}

	switch (textureIndex)
	{
	case 0: return gSceneTextures[0].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 1: return gSceneTextures[1].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 2: return gSceneTextures[2].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 3: return gSceneTextures[3].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 4: return gSceneTextures[4].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 5: return gSceneTextures[5].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 6: return gSceneTextures[6].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 7: return gSceneTextures[7].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 8: return gSceneTextures[8].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 9: return gSceneTextures[9].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 10: return gSceneTextures[10].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 11: return gSceneTextures[11].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 12: return gSceneTextures[12].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 13: return gSceneTextures[13].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 14: return gSceneTextures[14].SampleLevel(gSampler, uv, 0.0f).rgb;
	case 15: return gSceneTextures[15].SampleLevel(gSampler, uv, 0.0f).rgb;
	default: return fallbackColor;
	}
}

float4 RgbaToFloat4(float r, float g, float b, float a)
{
	return float4(clamp01(r), clamp01(g), clamp01(b), clamp01(a));
}

// Simple pseudo-random number generator
float random(uint seed)
{
	seed = (seed ^ 61u) ^ (seed >> 16u);
	seed *= 0x27d4eb2du;
	seed = seed ^ (seed >> 15u);
	return float(seed & 0x7FFFFFFFu) / 2147483647.0f;
}

// Sphere intersection
bool IntersectSphere(float3 origin, float3 ray, inout float distance, out float3 hitPoint, in Sphere sphere)
{
	float3 oc = origin - sphere.Position;
	float a = dot(ray, ray);
	float b = 2.0f * dot(oc, ray);
	float c = dot(oc, oc) - sphere.Radius * sphere.Radius;
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
		return false;

	float t = (-b - sqrt(discriminant)) / (2.0f * a);
	if (t < 0.0001f || t >= distance)
		return false;

	distance = t;
	hitPoint = origin + ray * t;
	return true;
}

// Quad intersection (simplified triangle-based)
bool IntersectQuad(float3 origin, float3 ray, inout float distance, out float3 hitPoint, in Quad quad)
{
	float3 v1 = quad.b - quad.a;
	float3 v2 = quad.d - quad.a;
	float3 normal = quad.Normal;

	float denom = dot(ray, normal);
	if (abs(denom) < 0.0001f)
		return false;

	float t = dot(quad.a - origin, normal) / denom;
	if (t < 0.0001f || t >= distance)
		return false;

	float3 p = origin + ray * t;
	float3 v = p - quad.a;

	float d1 = dot(v1, v1);
	float d2 = dot(v2, v2);
	float d1v = dot(v1, v);
	float d2v = dot(v2, v);

	float denom2 = d1 * d2 - dot(v1, v2) * dot(v1, v2);
	if (abs(denom2) < 0.0001f)
		return false;

	float u = (d2 * d1v - dot(v1, v2) * d2v) / denom2;
	float v_val = (d1 * d2v - dot(v1, v2) * d1v) / denom2;

	if (u < 0.0f || u > 1.0f || v_val < 0.0f || v_val > 1.0f)
		return false;

	distance = t;
	hitPoint = p;
	return true;
}

float3 TraceSecondary(float3 origin, float3 ray)
{
	// Iterative (non-recursive) path tracing up to a fixed maximum depth
	float3 accumColor = float3(0, 0, 0);
	float3 throughput = float3(1, 1, 1);
	static const uint kMaxTraceDepth = 8;

	[unroll]
	for (uint depth = 0; depth < kMaxTraceDepth; ++depth)
	{
		if (depth >= gMaxDepth)
		{
			break;
		}

		float distance = 1e9;
		int hitType = -1;
		int hitIndex = -1;
		float3 hitPoint = float3(0,0,0);
		float3 normal = float3(0,0,0);
		float3 baseColor = float3(0,0,0);
		float reflectivity = 0.0f;
		float refractivity = 0.0f;
		float eta = 1.0f;

		// Find nearest intersection
		for (uint i = 0; i < gNumSpheres; ++i)
		{
			Sphere sphere = gSpheres[i];
			float3 testHit;
			if (IntersectSphere(origin, ray, distance, testHit, sphere))
			{
				hitPoint = testHit;
				hitType = 0;
				hitIndex = i;
			}
		}

		for (uint i = 0; i < gNumQuads; ++i)
		{
			Quad quad = gQuads[i];
			float3 testHit;
			if (IntersectQuad(origin, ray, distance, testHit, quad))
			{
				hitPoint = testHit;
				hitType = 1;
				hitIndex = i;
			}
		}

		for (uint i = 0; i < gNumLights; ++i)
		{
			Light light = gLights[i];
			if (light.IsSphere)
			{
				Sphere sphere = gSpheres[light.ObjectIndex];
				float3 testHit;
				float testDist = distance;
				if (IntersectSphere(origin, ray, testDist, testHit, sphere))
				{
					hitPoint = testHit;
					distance = testDist;
					hitType = 2;
					hitIndex = i;
				}
			}
		}

		if (hitType == -1)
		{
			// Miss: add environment and terminate
			accumColor += throughput * float3(0.1f, 0.1f, 0.15f);
			break;
		}

		if (hitType == 0)
		{
			Sphere sphere = gSpheres[hitIndex];
			normal = normalize(hitPoint - sphere.Position);
			float2 sphereUV = float2(atan2(normal.z, normal.x) * 0.15915494309f + 0.5f, acos(clamp(normal.y, -1.0f, 1.0f)) * 0.31830988618f);
			baseColor = SampleSceneTexture(sphere.TextureIndex, sphereUV, sphere.Color);
			reflectivity = sphere.Reflection;
			refractivity = sphere.Refraction;
			eta = sphere.Eta;

			// local shading
			float3 local = baseColor * 0.1f;
			for (uint i = 0; i < gNumLights; ++i)
			{
				Light light = gLights[i];
				float3 toLight = normalize(light.Position - hitPoint);
				float diff = max(0.0f, dot(normal, toLight));
				local += baseColor * diff * light.Diffuse;
			}

			// Add base contribution scaled by how much is not reflected/refracted
			float occ = max(reflectivity, refractivity);
			accumColor += throughput * local * (1.0f - occ);

			// Decide to continue with reflection or refraction
			if (reflectivity > refractivity && reflectivity > 0.0f)
			{
				throughput *= reflectivity;
				ray = reflect_vec(ray, normal);
				origin = hitPoint + normal * 0.001f;
				continue;
			}
			else if (refractivity > 0.0f)
			{
				throughput *= refractivity;
				ray = refract_vec(ray, normal, eta);
				origin = hitPoint - normal * 0.001f;
				continue;
			}
			else
			{
				break;
			}
		}
		else if (hitType == 1)
		{
			Quad quad = gQuads[hitIndex];
			normal = quad.Normal;
			float2 quadUV = float2(dot(hitPoint - quad.a, quad.Tangent), dot(hitPoint - quad.a, quad.Binormal));
			baseColor = SampleSceneTexture(quad.TextureIndex, quadUV, quad.Color);
			reflectivity = quad.Reflection;

			float3 local = baseColor * 0.1f;
			for (uint i = 0; i < gNumLights; ++i)
			{
				Light light = gLights[i];
				float3 toLight = normalize(light.Position - hitPoint);
				float diff = max(0.0f, dot(quad.Normal, toLight));
				local += baseColor * diff * light.Diffuse;
			}

			accumColor += throughput * local * (1.0f - reflectivity);

			if (reflectivity > 0.0f)
			{
				throughput *= reflectivity;
				ray = reflect_vec(ray, quad.Normal);
				origin = hitPoint + quad.Normal * 0.001f;
				continue;
			}
			else
			{
				break;
			}
		}
		else if (hitType == 2)
		{
			Light light = gLights[hitIndex];
			accumColor += throughput * light.Color;
			break;
		}
	}

	return accumColor;
}

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint x = dispatchThreadID.x;
	uint y = dispatchThreadID.y;

	if (x >= gWidth || y >= gHeight)
	{
		return;
	}

	float2 uv = float2((float(x) / max(1.0f, float(gWidth - 1))) * 2.0f - 1.0f,
		(float(y) / max(1.0f, float(gHeight - 1))) * 2.0f - 1.0f);
	float4 transformedRay = mul(float4(uv.x, uv.y, 0.0f, 1.0f), gCameraRayMatrix);
	float3 rayDir = normalize_safe(transformedRay.xyz);

	float3 color = TraceSecondary(gCameraPosition, rayDir);

	gOutput[uint2(x, y)] = RgbaToFloat4(color.x, color.y, color.z, 1.0f);
}
