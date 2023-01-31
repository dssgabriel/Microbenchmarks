/* This is a one-off microbenchmark for attempts to dissect
 * Zhaoxin's KX-6640MA (LuJiaZui) architecture
 */
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// make mingw happy for cross compiling
#ifdef __MINGW32__
#define aligned_alloc(align, size) _aligned_malloc(size, align)
#endif

extern uint64_t clktest(uint64_t iterations, void *data);

extern uint64_t noptest(uint64_t iterations, void *data);
extern uint64_t addtest(uint64_t iterations, void *data);
extern uint64_t faddtest(uint64_t iterations, void *data);
extern uint64_t faddlattest(uint64_t iterations, void *data);
extern uint64_t vaddtest(uint64_t iterations, void *data);
extern uint64_t vaddlattest(uint64_t iterations, void *data);
extern uint64_t xaddtest(uint64_t iterations, void *data);
extern uint64_t xaddlattest(uint64_t iterations, void *data);
extern uint64_t multest(uint64_t iterations, void *data);
extern uint64_t mullattest(uint64_t iterations, void *data);
extern uint64_t fmultest(uint64_t iterations, void *data);
extern uint64_t fmullattest(uint64_t iterations, void *data);
extern uint64_t vmultest(uint64_t iterations, void *data);
extern uint64_t vmullattest(uint64_t iterations, void *data);
extern uint64_t xmultest(uint64_t iterations, void *data);
extern uint64_t xmullattest(uint64_t iterations, void *data);
extern uint64_t divtest(uint64_t iterations, void *data);
extern uint64_t divlattest(uint64_t iterations, void *data);
extern uint64_t fdivtest(uint64_t iterations, void *data);
extern uint64_t fdivlattest(uint64_t iterations, void *data);
extern uint64_t fmatest(uint64_t iterations, void *data);
extern uint64_t fmalattest(uint64_t iterations, void *data);
extern uint64_t vfmatest(uint64_t iterations, void *data);
extern uint64_t vfmalattest(uint64_t iterations, void *data);
extern uint64_t xfmatest(uint64_t iterations, void *data);
extern uint64_t xfmalattest(uint64_t iterations, void *data);
extern uint64_t fadd_fmultest(uint64_t iterations, void *data);
extern uint64_t fadd_multest(uint64_t iterations, void *data);
extern uint64_t fadd_mul_multest(uint64_t iterations, void *data);
extern uint64_t fadd_fmul_fmultest(uint64_t iterations, void *data);
extern uint64_t fma_faddtest(uint64_t iterations, void *data);
extern uint64_t fma_fmultest(uint64_t iterations, void *data);
extern uint64_t fma_fmul21test(uint64_t iterations, void *data);
extern uint64_t fma_fmul_faddtest(uint64_t iterations, void *data);
extern uint64_t loadtest(uint64_t iterations, void *data);
extern uint64_t storetest(uint64_t iterations, void *data);
extern uint64_t load_storetest(uint64_t iterations, void *data);

float fpTestArr[8] __attribute__ ((aligned (64))) = { 0.2, 1.5, 2.7, 3.14, 5.16, 6.3, 7.7, 9.45 };
float fpSinkArr[8] __attribute__ ((aligned (64))) = { 2.1, 3.2, 4.3, 5.4, 6.2, 7.8, 8.3, 9.4 };
int *intTestArr;
int intSinkArr[8] __attribute__ ((aligned (64))) = { 2, 3, 4, 5, 6, 7, 8, 9 };

float measureFunction(uint64_t iterations, float clockSpeedGhz, void *arr, uint64_t (*testfunc)(uint64_t, void *));

int main(int argc, char *argv[]) {
  struct timeval startTv, endTv;
  struct timezone startTz, endTz;
  uint64_t iterations = 500000000;
  uint64_t iterationsHigh = iterations * 5;
  uint64_t time_diff_ms;
  float latency, opsPerNs, clockSpeedGhz;
  uint64_t intTestArrLength = 1024;

  intTestArr = aligned_alloc(64, sizeof(int) * intTestArrLength);
  for (uint64_t i = 0; i < intTestArrLength; i++) {
    intTestArr[i] = i;
  }

  if (argc > 2) {
    uint64_t scaleFactor = (uint64_t)atol(argv[2]);
    iterationsHigh =  iterationsHigh * scaleFactor;
    iterations *= scaleFactor;
    printf("setting %lu iterations\n", iterationsHigh);
  }

  //if (argc == 1) {
    // figure out clock speed
    gettimeofday(&startTv, &startTz);
    clktest(iterationsHigh, NULL);
    gettimeofday(&endTv, &endTz);
    time_diff_ms = 1000 * (endTv.tv_sec - startTv.tv_sec) + ((endTv.tv_usec - startTv.tv_usec) / 1000);
    latency = 1e6 * (float)time_diff_ms / (float)iterationsHigh;
    // clk speed should be 1/latency, assuming we got one add per clk, roughly
    clockSpeedGhz = 1/latency;
  //} else clockSpeedGhz = 1.0f;

  printf("Estimated clock speed: %.2f GHz\n", clockSpeedGhz);

  // throughput
  if (argc == 1 || argc > 1 && strncmp(argv[1], "nop", 3) == 0)
    printf("nops per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, noptest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "add", 3) == 0)
    printf("int adds per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, addtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "fadd", 3) == 0)
    printf("fadds per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, faddtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "faddlatency", 3) == 0)
    printf("fadd latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, faddlattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "vadd", 3) == 0)
    printf("128b adds per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, vaddtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "vaddlatency", 3) == 0)
    printf("128b add latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, vaddlattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "xadd", 3) == 0)
    printf("256b adds per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, xaddtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "xaddlatency", 3) == 0)
    printf("256b add latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, xaddlattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "mul", 3) == 0)
    printf("int muls per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, intTestArr, multest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "mullatency", 3) == 0)
    printf("int mul latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, mullattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "fmul", 3) == 0)
    printf("fmuls per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fmultest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "fmullatency", 3) == 0)
    printf("fmul latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, fmullattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "vmul", 3) == 0)
    printf("128b muls per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, vmultest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "vmullatency", 3) == 0)
    printf("128b mul latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, vmullattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "xmul", 3) == 0)
    printf("256b muls per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, xmultest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "vmullatency", 3) == 0)
    printf("256b mul latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, xmullattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "div", 3) == 0)
    printf("divs per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, divtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "divlatency", 3) == 0)
    printf("int div latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, divlattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "fdiv", 3) == 0)
    printf("fdivs per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fdivtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "fdivlatency", 3) == 0)
    printf("fdiv latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, fdivlattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "fma", 3) == 0)
    printf("fmas per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, fpTestArr, fmatest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "fmalatency", 3) == 0)
    printf("fma latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, fmalattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "vfma", 3) == 0)
    printf("128b fmas per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, vfmatest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "vfmalatency", 3) == 0)
    printf("128b fma latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, vfmalattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "xfma", 3) == 0)
    printf("256b fmas per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, xfmatest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "xfmalatency", 3) == 0)
    printf("256b fma latency in cycles: %.2f\n", 1/measureFunction(iterationsHigh, clockSpeedGhz, NULL, xfmalattest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:1 fadd:fmul", 13) == 0)
    printf("1:1 fadd:fmul per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fadd_fmultest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:1 fadd:mul", 12) == 0)
    printf("1:1 fadd:mul per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fadd_multest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:2 fadd:mul", 12) == 0)
    printf("1:2 fadd:mul per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fadd_mul_multest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:2 fadd:fmul", 13) == 0)
    printf("1:2 fadd:fmul per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fadd_fmul_fmultest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:1 fma:fadd", 12) == 0)
    printf("1:1 64-bit fma:fadd per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fma_faddtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:1 fma:fmul", 12) == 0)
    printf("1:1 64-bit fma:fmul per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, fpTestArr, fma_fmultest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "2:1 fma:fmul", 12) == 0)
    printf("2:1 64-bit fma:fmul per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, fpTestArr, fma_fmul21test));
  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:1:1 fma:fmul:fadd", 17) == 0)
    printf("1:1:1 fma:fmul:fadd per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, fma_fmul_faddtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "load", 4) == 0)
    printf("load per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, intSinkArr, loadtest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "store", 4) == 0)
    printf("store per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, intSinkArr, storetest));

  if (argc == 1 || argc > 1 && strncmp(argv[1], "1:1 load:store", 4) == 0)
    printf("1:1 load:store per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, intSinkArr, load_storetest));
  return 0;
}

float measureFunction(uint64_t iterations, float clockSpeedGhz, void *arr, uint64_t (*testfunc)(uint64_t, void *)) {
  struct timeval startTv, endTv;
  struct timezone startTz, endTz;
  uint64_t time_diff_ms, retval;
  float latency, opsPerNs;

  gettimeofday(&startTv, &startTz);
  retval = testfunc(iterations, arr);
  gettimeofday(&endTv, &endTz);
  time_diff_ms = 1000 * (endTv.tv_sec - startTv.tv_sec) + ((endTv.tv_usec - startTv.tv_usec) / 1000);
  latency = 1e6 * (float)time_diff_ms / (float)iterations;
  opsPerNs = 1/latency;
  //printf("%f adds/ns, %f adds/clk?\n", opsPerNs, opsPerNs / clockSpeedGhz);
  //printf("return value: %lu\n", retval);
  return opsPerNs / clockSpeedGhz;
}
