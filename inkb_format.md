# INKB File Format

## Type Formats

### Int8
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 1 | Int8 | Byte |

### Int16
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 2 | Int8[2] | Bytes 0-1 |

### Int32
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 4 | Int8[4] | Bytes 0-3 |

### Int64
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 8 | Int8[8] | Bytes 0-7 |

### Float
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 4 | Int8[4] | Float bytes 0-3 |

### Double
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 8 | Int8[8] | Double bytes 0-7 |

### String
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 2 | Int16 | String length |
| 2 | X | Int8[X] | String contents |

### Vector
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 2 | Int16 | Vector length |
| 2 | X | T[X] | Vector contents |

## File Format
| Offset | Size | Type | Description |
| ------ | ---- | ---- | ----------- |
| 0 | 4 | N/A | File header; always "INKB" |
| 4 | 1 | int8 | INKB file format version |
| 5 | 2 | int16 | Number of knots in the story |
