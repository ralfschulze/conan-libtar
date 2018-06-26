/**
 * Test that heavily borrows from sample found in:
 * https://github.com/tklauser/libtar/blob/master/libtar/libtar.c
 */

#include <libtar.h>

#include <fstream>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <bsd/string.h>
#include <unistd.h>

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif


char *progname;
int verbose = 0;
int use_gnu = 0;

#ifdef HAVE_LIBZ

int
gzopen_frontend(char *pathname, int oflags, int mode)
{
	std::string gzoflags;
	gzFile gzf;
	int fd;

	switch (oflags & O_ACCMODE)
	{
	case O_WRONLY:
		gzoflags = "wb";
		break;
	case O_RDONLY:
		gzoflags = "rb";
		break;
	default:
	case O_RDWR:
		errno = EINVAL;
		return -1;
	}

	fd = open(pathname, oflags, mode);
	if (fd == -1)
		return -1;

	if ((oflags & O_CREAT) && fchmod(fd, mode))
	{
		close(fd);
		return -1;
	}

	gzf = gzdopen(fd, gzoflags.c_str());
	if (!gzf)
	{
		errno = ENOMEM;
		return -1;
	}

	/* This is a bad thing to do on big-endian lp64 systems, where the
	   size and placement of integers is different than pointers.
	   However, to fix the problem 4 wrapper functions would be needed and
	   an extra bit of data associating GZF with the wrapper functions.  */
	return *reinterpret_cast<int*>(reinterpret_cast<void*>(&gzf));
}

tartype_t gztype = { (openfunc_t) gzopen_frontend, (closefunc_t) gzclose,
	(readfunc_t) gzread, (writefunc_t) gzwrite
};

#endif /* HAVE_LIBZ */


int
create(char *tarfile, char *rootdir, libtar_list_t *l)
{
	TAR *t;
	char *pathname;
	char buf[MAXPATHLEN];
	libtar_listptr_t lp;

	if (tar_open(&t, tarfile,
#ifdef HAVE_LIBZ
		     &gztype,
#else
		     NULL,
#endif
		     O_WRONLY | O_CREAT, 0644,
		     (verbose ? TAR_VERBOSE : 0)
		     | (use_gnu ? TAR_GNU : 0)) == -1)
	{
		fprintf(stderr, "tar_open(): %s\n", strerror(errno));
		return -1;
	}

	libtar_listptr_reset(&lp);
	while (libtar_list_next(l, &lp) != 0)
	{
		pathname = (char *)libtar_listptr_data(&lp);
		if (pathname[0] != '/' && rootdir != NULL)
			snprintf(buf, sizeof(buf), "%s/%s", rootdir, pathname);
		else
			strlcpy(buf, pathname, sizeof(buf));
		if (tar_append_tree(t, buf, pathname) != 0)
		{
			fprintf(stderr,
				"tar_append_tree(\"%s\", \"%s\"): %s\n", buf,
				pathname, strerror(errno));
			tar_close(t);
			return -1;
		}
	}

	if (tar_append_eof(t) != 0)
	{
		fprintf(stderr, "tar_append_eof(): %s\n", strerror(errno));
		tar_close(t);
		return -1;
	}

	if (tar_close(t) != 0)
	{
		fprintf(stderr, "tar_close(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
#ifdef HAVE_LIBZ
	char tarfile[] = "test.tar.gz";
#else
    char tarfile[] = "test.tar";
#endif
    libtar_list_t *l = libtar_list_new(LIST_QUEUE, NULL);

    std::ofstream foo("/tmp/foo.txt");
    foo << "foo";
    foo.close();

    std::ofstream bar("/tmp/bar.txt");
    bar << "bar";
    bar.close();

    libtar_list_add(l, const_cast<char*>("foo.txt"));
    libtar_list_add(l, const_cast<char*>("bar.txt"));
    int return_code = create(tarfile, const_cast<char*>("/tmp"), l);
    libtar_list_free(l, NULL);

	return return_code;
}
