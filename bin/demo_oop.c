#include <stdio.h>

#include "prelude.h"

/** Base class methods. */
struct PersonOperations;

/** Base class. */
struct Person {
  struct PersonOperations *ops;
  char *name;
  int age;
};

struct PersonOperations {
  void (*say_hello)(struct Person *self);
};

void person_say_hello(struct Person *self) {
  printf("Hello, my name is %s, I'm %d years old.\n", self->name, self->age);
}
/** Static vtable of the base class. */
static struct PersonOperations person_ops = {
  .say_hello = person_say_hello,
};

/** Constructor of the base class. */
struct Person *person_new(char *name, int age) {
  struct Person *self = emalloc(sizeof(struct Person));
  self->ops = &person_ops;
  self->name = name;
  self->age = age;
  return self;
}

/** Destructor of the base class. */
void person_free(struct Person *self) {
  free(self);
}

/** Derived class. */
struct Student {
  struct Person person;
  char *school;
};

/** Override the method of the base class. */
void student_say_hello(struct Person *self) {
  struct Student *student = container_of(self, struct Student, person);
  printf("Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
         student->person.name, student->person.age, student->school);
}

/** Static vtable of the derived class. */
static struct PersonOperations student_ops = {
  .say_hello = student_say_hello,
};

/** Constructor of the derived class. */
struct Student *student_new(char *name, int age, char *school) {
  struct Student *self = emalloc(sizeof(struct Student));
  self->person.ops = &student_ops;
  self->person.name = name;
  self->person.age = age;
  self->school = school;
  return self;
}

/** Destructor of the derived class. */
void student_free(struct Student *self) {
  free(self);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(struct Person *, person_free);
DEFINE_TRIVIAL_CLEANUP_FUNC(struct Student *, student_free);
#define _cleanup_person_  _cleanup_(person_freep)
#define _cleanup_student_ _cleanup_(student_freep)

int main(_unused_ int argc, _unused_ char *argv[]) {
  _cleanup_person_ struct Person *alice = person_new("Alice", 20);
  _cleanup_person_ struct Person *bob = person_new("Bob", 21);
  _cleanup_student_ struct Student *carol = student_new("Carol", 22, "MIT");

  struct Person *people[] = {alice, bob, &carol->person};

  for (size_t i = 0; i < array_size(people); ++i) {
    apply(people[i], ops->say_hello);
  }

  return EXIT_SUCCESS;
}
