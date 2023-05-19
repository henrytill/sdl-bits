#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/// Base class methods.
typedef struct person_operations person_operations;

/// Base class.
typedef struct person person;

struct person_operations {
  void (*hello)(const person *self);
};

struct person {
  const person_operations *ops;
  char *name;
  int age;
  person *next;
};

static void person_hello(const person *self) {
  printf("Hello, my name is %s, I'm %d years old.\n", self->name, self->age);
}

/// Base class vtable.
static const person_operations PERSON_OPS = {
  .hello = person_hello,
};

/// Derived class.
typedef struct student {
  person person;
  char *school;
} student;

/// Derived class override of person_operations::hello
static void student_hello(const person *self) {
  const student *student = CONTAINER_OF(self, struct student, person);
  printf("Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
         student->person.name, student->person.age, student->school);
}

/// Derived class vtable.
static const person_operations STUDENT_OPS = {
  .hello = student_hello,
};

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  student carol = {
    .person = {
      .ops = &STUDENT_OPS,
      .name = "Carol",
      .age = 22,
      .next = NULL,
    },
    .school = "MIT",
  };
  person bob = {
    .ops = &PERSON_OPS,
    .name = "Bob",
    .age = 21,
    .next = &carol.person,
  };
  person alice = {
    .ops = &PERSON_OPS,
    .name = "Alice",
    .age = 20,
    .next = &bob,
  };

  for (const person *p = &alice; p != NULL; p = p->next) {
    SEND(p, ops->hello);
  }

  return EXIT_SUCCESS;
}
