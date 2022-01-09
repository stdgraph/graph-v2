// Includes csv_parser/single_include/csv.hpp and wraps it pragma to disable distracting warnings

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4458) // declaration of 'value' hides class member
#  pragma warning(disable : 4244) // conversion from 'double' to 'unsigned __int64', possible loss of data
#  pragma warning(                                                                                                     \
        disable : 4996) // 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#else
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wuseless-cast"
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include "single_include/csv.hpp"

#ifdef _MSC_VER
#  pragma warning(pop)
#else
#  pragma GCC diagnostic pop
#endif
