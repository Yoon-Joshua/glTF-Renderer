// #include "astc.h"

// #include <cstring>
// #include <mutex>

// #include "runtime/renderer/common/error.h"
// #include "runtime/renderer/common/glm_common.h"

// /*
// #if defined(_WIN32) || defined(_WIN64)
// // Windows.h defines IGNORE, so we must #undef it to avoid clashes with
// // astcheader
// #undef IGNORE
// #endif
// #include <astc_codec_internals.h>
// */
// #define MAGIC_FILE_CONSTANT 0x5CA1AB13

// namespace sg {
// BlockDim to_blockdim(const VkFormat format) {
//   switch (format) {
//     case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
//       return {4, 4, 1};
//     case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
//       return {5, 4, 1};
//     case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
//       return {5, 5, 1};
//     case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
//       return {6, 5, 1};
//     case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
//       return {6, 6, 1};
//     case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
//       return {8, 5, 1};
//     case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
//       return {8, 6, 1};
//     case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
//       return {8, 8, 1};
//     case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
//       return {10, 5, 1};
//     case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
//       return {10, 6, 1};
//     case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
//       return {10, 8, 1};
//     case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
//       return {10, 10, 1};
//     case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
//       return {12, 10, 1};
//     case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
//     case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
//       return {12, 12, 1};
//     default:
//       throw std::runtime_error{"Invalid astc format"};
//   }
// }

// struct AstcHeader {
//   uint8_t magic[4];
//   uint8_t blockdim_x;
//   uint8_t blockdim_y;
//   uint8_t blockdim_z;
//   uint8_t xsize[3];  // x-size = xsize[0] + xsize[1] + xsize[2]
//   uint8_t ysize[3];  // x-size, y-size and z-size are given in texels;
//   uint8_t zsize[3];  // block count is inferred
// };

// void Astc::init() {
//   printf("ERROR: ASTC is banned!\n");
//   exit(-1);

//   //   // Initializes ASTC library
//   //   static bool initialized{false};
//   //   static std::mutex initialization;
//   //   std::unique_lock<std::mutex> lock{initialization};
//   //   if (!initialized) {
//   //     // Init stuff
//   //     prepare_angular_tables();
//   //     build_quantization_mode_table();
//   //     initialized = true;
//   //   }
// }

// void Astc::decode(BlockDim blockdim, VkExtent3D extent, const uint8_t *data_) {
//   printf("ERROR: ASTC is banned!\n");
//   exit(-1);
//   //   // Actual decoding
//   //   astc_decode_mode decode_mode = DECODE_LDR_SRGB;
//   //   uint32_t bitness = 8;
//   //   swizzlepattern swz_decode = {0, 1, 2, 3};

//   //   int xdim = blockdim.x;
//   //   int ydim = blockdim.y;
//   //   int zdim = blockdim.z;

//   //   if ((xdim < 3 || xdim > 6 || ydim < 3 || ydim > 6 || zdim < 3 || zdim >
//   //   6) &&
//   //       (xdim < 4 || xdim == 7 || xdim == 9 || xdim == 11 || xdim > 12 ||
//   //        ydim < 4 || ydim == 7 || ydim == 9 || ydim == 11 || ydim > 12 ||
//   //        zdim != 1)) {
//   //     throw std::runtime_error{"Error reading astc: invalid block"};
//   //   }

//   //   int xsize = extent.width;
//   //   int ysize = extent.height;
//   //   int zsize = extent.depth;

//   //   if (xsize == 0 || ysize == 0 || zsize == 0) {
//   //     throw std::runtime_error{"Error reading astc: invalid size"};
//   //   }

//   //   int xblocks = (xsize + xdim - 1) / xdim;
//   //   int yblocks = (ysize + ydim - 1) / ydim;
//   //   int zblocks = (zsize + zdim - 1) / zdim;

//   //   auto astc_image = allocate_image(bitness, xsize, ysize, zsize, 0);
//   //   initialize_image(astc_image);

//   //   imageblock pb;
//   //   for (int z = 0; z < zblocks; z++) {
//   //     for (int y = 0; y < yblocks; y++) {
//   //       for (int x = 0; x < xblocks; x++) {
//   //         int offset = (((z * yblocks + y) * xblocks) + x) * 16;
//   //         const uint8_t *bp = data_ + offset;

//   //         physical_compressed_block pcb =
//   //             *reinterpret_cast<const physical_compressed_block *>(bp);
//   //         symbolic_compressed_block scb;

//   //         physical_to_symbolic(xdim, ydim, zdim, pcb, &scb);
//   //         decompress_symbolic_block(decode_mode, xdim, ydim, zdim, x *
//   //         xdim,
//   //                                   y * ydim, z * zdim, &scb, &pb);
//   //         write_imageblock(astc_image, &pb, xdim, ydim, zdim, x * xdim, y *
//   //         ydim,
//   //                          z * zdim, swz_decode);
//   //       }
//   //     }
//   //   }

//   //   set_data(astc_image->imagedata8[0][0],
//   //            astc_image->xsize * astc_image->ysize * astc_image->zsize *
//   //            4);
//   //   set_format(VK_FORMAT_R8G8B8A8_SRGB);
//   //   set_width(static_cast<uint32_t>(astc_image->xsize));
//   //   set_height(static_cast<uint32_t>(astc_image->ysize));
//   //   set_depth(static_cast<uint32_t>(astc_image->zsize));

//   //   destroy_image(astc_image);
// }

// Astc::Astc(const Image &image) : Image{image.get_name()} {
//   init();

//   // Locate mip #0 in the KTX. This is the first one in the data array for
//   // KTX1s, but the last one in KTX2s!
//   auto mip_it =
//       std::find_if(image.get_mipmaps().begin(), image.get_mipmaps().end(),
//                    [](auto &mip) { return mip.level == 0; });
//   assert(mip_it != image.get_mipmaps().end() && "Mip #0 not found");

//   // When decoding ASTC on CPU (as it is the case in here), we don't decode
//   // all mips in the mip chain. Instead, we just decode mip #0 and re-generate
//   // the other LODs later (via image->generate_mipmaps()).
//   const auto blockdim = to_blockdim(image.get_format());
//   const uint8_t *data_ptr = image.get_data().data() + mip_it->offset;
//   decode(blockdim, mip_it->extent, data_ptr);
// }

// Astc::Astc(const std::string &name, const std::vector<uint8_t> &data)
//     : Image{name} {
//   init();

//   // Read header
//   if (data.size() < sizeof(AstcHeader)) {
//     throw std::runtime_error{"Error reading astc: invalid memory"};
//   }
//   AstcHeader header{};
//   std::memcpy(&header, data.data(), sizeof(AstcHeader));
//   uint32_t magicval = header.magic[0] +
//                       256 * static_cast<uint32_t>(header.magic[1]) +
//                       65536 * static_cast<uint32_t>(header.magic[2]) +
//                       16777216 * static_cast<uint32_t>(header.magic[3]);
//   if (magicval != MAGIC_FILE_CONSTANT) {
//     throw std::runtime_error{"Error reading astc: invalid magic"};
//   }

//   BlockDim blockdim = {/* xdim = */ header.blockdim_x,
//                        /* ydim = */ header.blockdim_y,
//                        /* zdim = */ header.blockdim_z};

//   VkExtent3D extent = {
//       /* width  = */ static_cast<uint32_t>(
//           header.xsize[0] + 256 * header.xsize[1] + 65536 * header.xsize[2]),
//       /* height = */
//       static_cast<uint32_t>(header.ysize[0] + 256 * header.ysize[1] +
//                             65536 * header.ysize[2]),
//       /* depth  = */
//       static_cast<uint32_t>(header.zsize[0] + 256 * header.zsize[1] +
//                             65536 * header.zsize[2])};

//   decode(blockdim, extent, data.data() + sizeof(AstcHeader));
// }

// }  // namespace sg
