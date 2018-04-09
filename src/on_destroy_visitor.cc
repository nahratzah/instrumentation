#include <instrumentation/visitor.h>
#include <instrumentation/hierarchy.h>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <utility>

namespace instrumentation {
namespace {


std::mutex odv_mtx;
std::unique_ptr<visitor> odv_ptr;


} /* namespace instrumentation::<unnamed> */


auto visitor::on_destroy_visitor(std::unique_ptr<visitor> vptr)
-> std::unique_ptr<visitor> {
  std::lock_guard<std::mutex> lck{ odv_mtx };
  return std::exchange(odv_ptr, std::move(vptr));
}


auto hierarchy::visit_before_destroy_(const hierarchy& h) noexcept
-> void {
  try {
    std::lock_guard<std::mutex> lck{ odv_mtx };
    if (odv_ptr != nullptr) h.visit(*odv_ptr);
  } catch (const std::exception& ex) {
    std::cerr << "on-destroy visitor for hierarchy threw exception: " << ex.what() << "\n";
  } catch (...) {
    std::cerr << "on-destroy visitor for hierarchy threw an exception\n";
  }
}


} /* namespace instrumentation */
