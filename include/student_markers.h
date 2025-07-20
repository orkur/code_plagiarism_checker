#ifndef STUDENT_MARKERS_H
#define STUDENT_MARKERS_H

#define STUDENT_START [[maybe_unused]] __attribute__((annotate("start_student_code"))) static int
#define STUDENT_END   [[maybe_unused]] __attribute__((annotate("end_student_code"))) static int

#endif