#include <uk/test.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>

typedef struct vfscore_stat {
    int rc;
    int errcode;
    char *filename;
} vfscore_stat_t;

static vfscore_stat_t test_stats [] = {
    { .rc = 0,    .errcode = 0,        .filename = "/foo/file.txt" },
    { .rc = -1,    .errcode = EINVAL,    .filename = NULL },
};

static int fd;

UK_TESTCASE(vfscore_stat_testsuite, vfscore_test_newfile)
{
    /* First check if mount works all right */
    int ret = mount("", "/", "ramfs", 0, NULL);
    UK_TEST_EXPECT_SNUM_EQ(ret, 0);

    ret = mkdir("/foo", S_IRWXU);
    UK_TEST_EXPECT_SNUM_EQ(ret, 0);

    fd = open("/foo/file.txt", O_WRONLY | O_CREAT, S_IRWXU);
    UK_TEST_EXPECT_SNUM_GT(fd, 2);

    UK_TEST_EXPECT_SNUM_EQ(
        write(fd, "hello\n", sizeof("hello\n")),
        sizeof("hello\n")
    );
    fsync(fd);
}

/* Register the test suite */
uk_testsuite_register(vfscore_stat_testsuite, NULL);
