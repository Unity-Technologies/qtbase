/* D3D Compiler stubs for Windows Phone
 *
 * These APIs are supported at development time, but are not part of the SDK.
 */

#ifndef WINPHONE_D3DCOMPILER_H
#define WINPHONE_D3DCOMPILER_H

#define D3DCOMPILE_DEBUG                          (1 << 0)
#define D3DCOMPILE_SKIP_VALIDATION                (1 << 1)
#define D3DCOMPILE_SKIP_OPTIMIZATION              (1 << 2)
#define D3DCOMPILE_PACK_MATRIX_ROW_MAJOR          (1 << 3)
#define D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR       (1 << 4)
#define D3DCOMPILE_PARTIAL_PRECISION              (1 << 5)
#define D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT       (1 << 6)
#define D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT       (1 << 7)
#define D3DCOMPILE_NO_PRESHADER                   (1 << 8)
#define D3DCOMPILE_AVOID_FLOW_CONTROL             (1 << 9)
#define D3DCOMPILE_PREFER_FLOW_CONTROL            (1 << 10)
#define D3DCOMPILE_ENABLE_STRICTNESS              (1 << 11)
#define D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY (1 << 12)
#define D3DCOMPILE_IEEE_STRICTNESS                (1 << 13)
#define D3DCOMPILE_OPTIMIZATION_LEVEL0            (1 << 14)
#define D3DCOMPILE_OPTIMIZATION_LEVEL1            0
#define D3DCOMPILE_OPTIMIZATION_LEVEL2            ((1 << 14) | (1 << 15))
#define D3DCOMPILE_OPTIMIZATION_LEVEL3            (1 << 15)
#define D3DCOMPILE_RESERVED16                     (1 << 16)
#define D3DCOMPILE_RESERVED17                     (1 << 17)
#define D3DCOMPILE_WARNINGS_ARE_ERRORS            (1 << 18)

#define D3DCOMPILER_DLL L"d3dcompiler_46.dll"

typedef long (__stdcall *pD3DCompile) (const void *, __int64, const char *, const D3D_SHADER_MACRO *,
                                       ID3DInclude *, const char *, const char *, unsigned int,
                                       unsigned int, ID3DBlob**, ID3DBlob**);

#endif // WINPHONE_D3DCOMPILER_H
