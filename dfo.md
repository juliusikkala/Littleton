# DarkFlowers Object File 0.1

This document specifies the structure of the file describing 3D objects for the
DarkFlowers engine. The format has been designed to support scene graphs and
physically based material definitions with high performance and low on-disk
memory usage. 

## 1. Definitions

### 1.1. Data Types

|    Name    |                          Definition                          |
|:----------:|:------------------------------------------------------------:|
|  F32       | Single-precision little-endian IEEE-754 floating-point value |
|  F64       | Double-precision little-endian IEEE-754 floating-point value |
|  U32       | Unsigned little-endian 32-bit integer                        |
|  U64       | Unsigned little-endian 64-bit integer                        |
|  I32       | Signed little-endian 32-bit integer                          |
|  I64       | Signed little-endian 32-bit integer                          |
|  RGBA      | 4 bytes, each representing red, green, blue and alpha        |
|  CHAR      | An ascii character                                           |
|  TYPE(LEN) | An array of values of TYPE with LEN elements                 |

## 2. File Structure

All padding is explicitly marked, everything is tightly packed by default.

### 2.1 Header

#### 2.1.1 Structure

|              Type          |          Name         |               Value |
|---------------------------:|-----------------------|--------------------:|
|                    CHAR(8) | magic                 |          "DFLOWERS" |
|                        U64 | length                |     total file size |
|                        U32 | version               |                   0 |
|                        U32 | texture\_table\_size  |  number of textures |
|  U32(texture\_table\_size) | texture\_table        |   array of textures |
|                        U32 | material\_table\_size | number of materials |
| U32(material\_table\_size) | material\_table       |  array of materials |
|                        U32 | object\_table\_size   |   number of objects |
|   U32(object\_table\_size) | object\_table         |    array of objects |

#### 2.1.2 Description

The header is placed at the beginning of the file.

'magic' is used for determining that the file is indeed a DarkFlowers Object
file. 'length' determines the total size of the file in bytes. 'version' is 
the version of the format specification, which should be set to 0.

'texture\_table', 'material\_table' and 'object\_table' define addresses for
texture, material and object entries respectively.

Textures are defined in chapter 2.5, materials in 2.2 and objects in 2.3.

Objects in 'object\_table' must be sorted such that parent objects always have
lower indices than their child objects.

### 2.2 Material

#### 2.2.1 Structure

|            Type |          Name          |                                Value |
|----------------:|------------------------|-------------------------------------:|
|             U32 | name\_len              | Length of the name field             |
| CHAR(name\_len) | name                   | Name of the material                 |
|          (none) | (padding)              | Zero padding aligning address to 4   |
|             U32 | type                   | Type of the material (see 2.2.2)     |
|      F32 or I32 | metallic               | Whether the material is metallic     |
|     RGBA or I32 | color                  | Color of the surface                 |
|      F32 or I32 | roughness              | Roughness of the surface             |
|             F32 | ior                    | Index of refraction of the volume    |
|             I32 | normal                 | Normal map of the surface            |
|      F32 or I32 | emission               | Emittance of the surface             |
|     RGBA or I32 | subsurface\_scattering | Amount of light scattered subsurface |
|      F32 or I32 | subsurface\_depth      | Total depth of the subsurface        |

#### 2.2.2 Description

The 'type' field selects between textures and constants for material values:

```
MATERIAL_CONSTANT = 0
MATERIAL_TEXTURE  = 1
```

| Bit |         Affected field |
|----:|-----------------------:|
|   0 |               metallic |
|   1 |                  color |
|   2 |              roughness |
|   3 |               emission |
|   4 | subsurface\_scattering |
|   5 |      subsurface\_depth |

Bits 6..31 must be initialized to zero.

If the relevant bit in 'type' is set to MATERIAL\_TEXTURE, the type of the
field is I32 and is an index to the texture table. If the index is -1, the
field is assumed to be undefined/unused.

If type[0] == MATERIAL\_CONSTANT, 'metallic' is an F32 in the range [0, 1],
describing the "metallicness" of the surface. 0 is dielectric, 1 is metallic.
If type[0] == MATERIAL\_TEXTURE, the texture must have only one channel, where
the pixel value describes the "metallicness" of the surface.

If type[1] == MATERIAL\_CONSTANT, 'color' is an RGBA color value. The alpha
channel determines the transparency of the surface. 0 is opaque,
1 is transparent. If type[1] == MATERIAL\_TEXTURE, the texture defines the
color of the surface.

If type[2] == MATERIAL\_CONSTANT, 'roughness' is an F32 in the range [0, 1],
describing the roughness of the surface. 0 is smooth, 1 is rough.
If type[2] == MATERIAL\_TEXTURE, the texture must have only one channel, where
the pixel value describes the roughness of the surface.

If type[3] == MATERIAL\_CONSTANT, 'emission' is an F32 in the range [0, +inf),
describing how emissive the surface is. 0 doesn't emit light, positive values
describe the emittance in lux. If type[3] == MATERIAL\_TEXTURE, the texture must
have only one channel, where the pixel value determines the emittance.

If type[4] == MATERIAL\_CONSTANT, 'subsurface\_scattering' is an RGBA value,
where the RGB part describes the color of the light scattered and determines
the fraction of light passing the surface of the material. If
type[4] == MATERIAL\_TEXTURE, the texture defines the color and fraction of 
light scattered, per pixel.

If type[5] == MATERIAL\_CONSTANT, 'subsurface\_depth' is an F32 in the range
[0, +inf). It is the depth of the subsurface. type[5] == MATERIAL\_TEXTURE, the
texture must have only one channel, where the pixel value determines the depth.

### 2.3 Object

#### 2.3.1 Structure

|              Type |     Name     |                              Value |
|------------------:|--------------|-----------------------------------:|
|               U32 | name\_len    | Length of the name field           |
|   CHAR(name\_len) | name         | Name of the object                 |
|            (none) | (padding)    | Zero padding aligning address to 4 |
|               I32 | parent       | Index of parent object             |
|           F32(16) | transform    | 4x4 transformation matrix          |
|               U32 | group\_count | Number of vertex groups            |
| U32(group\_count) | group\_table | Table of group addresses           |

#### 2.3.2 Description

All objects together form the scene graph. When transforming an object into
world coordinates, all parent transformations are applied in addition to the
'transform' field. Objects consist of several vertex groups. Vertex groups are
defined in chapter 2.4.

The 'transform' matrix is laid out in memory as column-major. If 'parent' is
-1, the object is a root object with no parents.

### 2.4 Vertex Group

#### 2.4.1 Structure

|                          Type |      Name     |                              Value |
|------------------------------:|---------------|-----------------------------------:|
|                           U32 | material\_id  | Index of the material of the group |
|                           U32 | vertex\_type  | Type of the vertices               |
|                           U32 | vertex\_count | Number of vertices                 |
| F32((3 or 5) * vertex\_count) | vertex\_data  | Vertex data (x, y, z and u, v)     |
|                           U32 | index\_count  | Number of indices                  |
|             U32(index\_count) | index\_data   | Indices into vertex\_data          |

#### 2.4.2 Description

vertex\_type can have the following values:

| Value |    Meaning    |
|------:|---------------|
|     0 | VERTEX\_XYZ   |
|     1 | VERTEX\_XYZUV |

vertex\_data entries are affected by the vertex\_type.
If vertex\_type == VERTEX\_XYZ, vertex\_data consists of object-space
coordinates only. If vertex\_type == VERTEX\_XYZUV, each vertex consists of
object-space coordinates and texture-space coordinates.

index\_data contains indices into vertex\_data, 3 indices per triangle. The
indices must be in the correct winding order.

### 2.5 Texture

#### 2.5.1 Structure

|            Type |    Name   |                              Value |
|----------------:|-----------|-----------------------------------:|
|             U32 | path\_len | Length of the path field           |
| CHAR(path\_len) | path      | Filesystem path of the texture     |
|          (none) | (padding) | Zero padding aligning address to 4 |

#### 2.5.2 Description

'path' is defined relative to this file, using Unix-style forward-slash paths.
