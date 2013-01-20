/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include <xenia/xenia.h>


using namespace xe;
using namespace xe::cpu;
using namespace xe::kernel;


class Run {
public:
  Run();
  ~Run();

  int Setup(const xechar_t* path);
  int Launch();

private:
  xe_pal_ref      pal_;
  xe_memory_ref   memory_;
  shared_ptr<Processor> processor_;
  shared_ptr<Runtime>   runtime_;
  UserModule*           module_;
};

Run::Run() {
}

Run::~Run() {
  xe_memory_release(memory_);
  xe_pal_release(pal_);
}

int Run::Setup(const xechar_t* path) {
  xe_pal_options_t pal_options;
  xe_zero_struct(&pal_options, sizeof(pal_options));
  pal_ = xe_pal_create(pal_options);
  XEEXPECTNOTNULL(pal_);

  xe_memory_options_t memory_options;
  xe_zero_struct(&memory_options, sizeof(memory_options));
  memory_ = xe_memory_create(pal_, memory_options);
  XEEXPECTNOTNULL(memory_);

  processor_ = shared_ptr<Processor>(new Processor(pal_, memory_));
  XEEXPECTZERO(processor_->Setup());

  runtime_ = shared_ptr<Runtime>(new Runtime(pal_, processor_, XT("")));

  XEEXPECTZERO(runtime_->LoadModule(path));
  module_ = runtime_->GetModule(path);

  return 0;
XECLEANUP:
  return 1;
}

int Run::Launch() {
  // TODO(benvanik): wait until the module thread exits
  runtime_->LaunchModule(module_);
  return 0;
}

int xenia_run(int argc, xechar_t **argv) {
  // Dummy call to keep the GPU code linking in to ensure it's working.
  do_gpu_stuff();

  int result_code = 1;

  // TODO(benvanik): real command line parsing.
  if (argc < 2) {
    printf("usage: xenia-run some.xex\n");
    return 1;
  }
  const xechar_t *path = argv[1];

  auto_ptr<Run> run = auto_ptr<Run>(new Run());

  result_code = run->Setup(path);
  XEEXPECTZERO(result_code);

  //xe_module_dump(run->module);

  run->Launch();

  return 0;

XECLEANUP:
  return result_code;
}
XE_MAIN_THUNK(xenia_run);
