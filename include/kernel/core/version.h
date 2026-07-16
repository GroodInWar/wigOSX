#ifndef KERNEL_CORE_VERSION_H
#define KERNEL_CORE_VERSION_H

/**
 * @file version.h
 * @brief Central version and stage information for wigOSX.
 *
 * Keep version/stage strings here so kernel output, shell commands,
 * and future diagnostics do not need to update hardcoded strings
 * across multiple files.
 */

#define WIGOSX_VERSION_STRING "0.015"

#define WIGOSX_STAGE_NUMBER_STRING "15"
#define WIGOSX_STAGE_NAME "Kernel boundary and failure-path hardening"

#define WIGOSX_STAGE_LABEL \
  "Stage " WIGOSX_STAGE_NUMBER_STRING ": " WIGOSX_STAGE_NAME

#endif /* KERNEL_CORE_VERSION_H */