#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/// Base class.
struct person;

/// Base class methods.
struct person_operations {
	void (*hello)(const struct person *self);
};

struct person {
	const struct person_operations *ops;
	char *name;
	int age;
	struct person *next;
};

static void person_hello(const struct person *self)
{
	(void)printf("Hello, my name is %s, I'm %d years old.\n",
		self->name, self->age);
}

/// Base class vtable.
static const struct person_operations PERSON_OPS = {
	.hello = person_hello,
};

/// Derived class.
struct student {
	struct person person;
	char *school;
};

/// Derived class override of person_operations::hello
static void student_hello(const struct person *self)
{
	const struct student *student = CONTAINER_OF(self, struct student, person);
	(void)printf("Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
		student->person.name, student->person.age, student->school);
}

/// Derived class vtable.
static const struct person_operations STUDENT_OPS = {
	.hello = student_hello,
};

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
	struct student carol = {
		.person = {
			.ops = &STUDENT_OPS,
			.name = "Carol",
			.age = 22,
			.next = NULL,
		},
		.school = "MIT",
	};
	struct person bob = {
		.ops = &PERSON_OPS,
		.name = "Bob",
		.age = 21,
		.next = &carol.person,
	};
	struct person alice = {
		.ops = &PERSON_OPS,
		.name = "Alice",
		.age = 20,
		.next = &bob,
	};

	for (const struct person *p = &alice; p != NULL; p = p->next) {
		SEND(p, ops->hello);
	}

	return EXIT_SUCCESS;
}
