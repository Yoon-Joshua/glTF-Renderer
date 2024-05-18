// #pragma once
// #include "runtime/renderer/scene_graph/components/image.h"
// namespace sg {

// struct BlockDim {
//   uint8_t x;
//   uint8_t y;
//   uint8_t z;
// };

// class Astc : public Image {
//  public:
//   /// @brief Decodes an ASTC image
//   /// @param image Image to decode
//   Astc(const Image &image);

//   /// @brief Decodes ASTC data with an ASTC header
//   /// @param name Name of the component
//   /// @param data ASTC data with header
//   Astc(const std::string &name, const std::vector<uint8_t> &data);

//   virtual ~Astc() = default;

//  private:
//   /**
//    * @brief Decodes ASTC data
//    * @param blockdim Dimensions of the block
//    * @param extent Extent of the image
//    * @param data Pointer to ASTC image data
//    */
//   void decode(BlockDim blockdim, VkExtent3D extent, const uint8_t *data);

//   /**
//    * @brief Initializes ASTC library
//    */
//   void init();
// };
// }  // namespace sg