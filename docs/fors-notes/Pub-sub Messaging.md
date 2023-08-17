One of fors' more unusual features is its publish/subscribe inter-process communication. It allows any process to create a number of publishers and subscribers to named topics, and perform some task asynchronously as a response to a message on a subscribed topic.

## Topics

Topics are named message channels which processes can publish and/or subscribe to. They follow a hierarchical naming structure, similar to filesystem paths. A topic name consists of some string separated by '/'s. The supported characters for a topic are: `a-z A-Z 0-9 _ . /`. '/' is only allowed when it acts as a separator between two strings. This is implied also by the next rule: topic names do not begin or end with a '/'.

Topic names must be at least 1 character long.

Some possible topic names are:
 - `apple`
 - `fruit_bowl/apple`
 - `a`
 - `__..`
 - `pstatus/0`

And some invalid ones are:
 - `/` ('/' is only valid as a separator between two strings)
 - `twitter_feed/@j4cobgarby` ('@' is not allowed)

Topics are not created directly, but implicitly when one or more processes have a publisher or subscriber to it. Topics, once created, are managed by the kernel, and not deleted even if all of its publishers and subscribers are closed.
## Publishers

To create a new publisher, a process can use the syscall `mkpub()`, which initialises a new publisher to a given topic. Publishing is done with the `publish()` syscall.

```c
int mkpub(const char *topic);
int publish(int pd, const void *data, int n);
int delpub(int pd);
```

`mkpub()` takes a topic name as a parameter, and if it succeeds to make a publisher then it returns a _publisher descriptor_, which is an integer >=0 which can be later used to refer to that particular publisher.

If the kernel fails to make the publisher, it returns a negative integer that represents the cause of error. See [[Error Codes]].

`publish()` publishes a piece of data to a topic, to which a publisher must already have been made by the calling process. The parameter `pd` is the publisher descriptor that was returned by `mkpub`. `data` is a pointer to the start of a region in memory that should be sent to the topic, and `n` is the size (in bytes) of this region.

After the process no longer needs the publisher, `delpub()` can be called, which makes the publisher descriptor no longer valid.

Usage example:

```c
// Publish two messages to two different topics.

int pub_a, pub_b;

const char *msg_1 = "Hello, world!";
const char *msg_2 = "This is another message.";

int main(int argc, char **argv) {
	if ((pub_a = mkpub("topic_a")) < 0) {
		return -1;
	}

	if ((pub_b = mkpub("sub/topic")) < 0) {
		return -1;
	}

	publish(pub_a, msg_1, sizeof(msg_1));
	publish(pub_b, msg_2, sizeof(msg_2));

	delpub(pub_a);
	delpub(pub_b);

	return 0;
}
```
## Subscribers

```c
// Create two subscribers attached to the same callback function

int sub_a, sub_b;

void callback(int pid, void *dat, int n) {
	printf("Process %d said '%s'\n", pid, dat);
}

int main(int argc, char **argv) {
	if ((sub_a = mksub("topic_a")) < 0) {
		return -1;
	}

	if ((sub_b = mksub("sub/topic")) < 0) {
		return -1;
	}

	subscribe(sub_a, callback, NULL);
	subscribe(sub_b, callback, NULL);

	while (1);
}
```