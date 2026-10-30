#pragma once

#include <exiv2/exiv2.hpp>
#include <exiv2/version.hpp>
#include <cstdint>

namespace AstroPhotoStacker {

  // Check whether Exiv2 version is >= 0.28.0
  #if (EXIV2_MAJOR_VERSION > 0) || \
      (EXIV2_MINOR_VERSION >= 28)

  #define EXIV2_NEW_API 1

  #else

  #define EXIV2_NEW_API 0

  #endif

  /**
   * @brief Compatibility layer for Exiv2 API differences between versions.
   */
  namespace Exiv2Compat {

    #if EXIV2_NEW_API

        // Exiv2 >= 0.28
        using ImagePtr = Exiv2::Image::UniquePtr;
        using Error = Exiv2::Error;
        #define Exiv2CompatErrorMessage Exiv2::ErrorCode::kerErrorMessage

    #else

        // Exiv2 < 0.28
        using ImagePtr = Exiv2::Image::AutoPtr;
        using Error = Exiv2::AnyError;
        #define Exiv2CompatErrorMessage Exiv2::kerErrorMessage

    #endif


        // Wrapper for converting metadata values to integers
        template<typename T>
        inline int64_t toInt64(const T& value)
        {
    #if EXIV2_NEW_API
            return value.toInt64();
    #else
            return value.toLong();
    #endif
        }

  }
}