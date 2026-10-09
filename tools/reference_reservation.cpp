// Test-only address reservation loaded before CRT initialization. No entry point.
// The exported zero-filled area keeps the fixed original image range occupied
// until ReferenceImage overlays it with the inventoried reference for comparison.
extern "C" __declspec(dllexport) unsigned char referenceReservationSpace[3*1024*1024]{};
extern "C" __declspec(dllexport) void* referenceReservationBase() {
    return reinterpret_cast<void*>(0x00400000U);
}
