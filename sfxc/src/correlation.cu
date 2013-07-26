__global__ void __autocorr(float2 *s, float2 *d, int n, int stride)
{
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	
	for (int j = i; j < n * stride; j += stride) {
	        d[i].x += s[j].x * s[j].x + s[j].y * s[j].y;
	        d[i].y += s[j].y * s[j].x - s[j].x * s[j].y;
	}
}

extern "C" void autocorr(float2 *s, float2 *d, int n, int stride)
{
	__autocorr<<<stride / 256, 256>>>(s, d, n, stride);
}

__global__ void __crosscorr(float2 *s1, float2 *s2, float2 *d, int n, int stride)
{
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	
	for (int j = i; j < n * stride; j += stride) {
	        d[i].x += s1[j].x * s2[j].x + s1[j].y * s2[j].y;
	        d[i].y += s1[j].y * s2[j].x - s1[j].x * s2[j].y;
	}
}

extern "C" void crosscorr(float2 *s1, float2 *s2, float2 *d, int n, int stride)
{
	__crosscorr<<<stride / 256, 256>>>(s1, s2, d, n, stride);
}
