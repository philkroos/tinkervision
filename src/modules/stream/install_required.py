import os, sys, apt, platform, subprocess, re

def replace_regex_in_file_line(filename, original, replacement, dry_run = False):
    if dry_run:
        print "Replacing", original, "with", replacement, "in", filename
        return
    with open(filename, "r") as input:
        lines = input.readlines()
    with open(filename, "w") as output:
        for line in lines:
            output.write(re.sub(r'' + original, replacement, line))


def execute(shell_cmd_string, directory = "", dry_run = False):
    cwd = directory if directory.startswith("/") else self.root + "/" + directory
    if not dry_run:
        return subprocess.call(shell_cmd_string, cwd = cwd, shell = True)
    else:
        print "Executing", shell_cmd_string, "in", cwd


def apt_get(package_name, clean = False, dry_run = False, prefix = None):
    apt_cache = apt.cache.Cache()
    # apt_cache = apt.Cache() # no difference
    print "Updating apt-cache..."
    apt_cache.update()
    try:
        package = apt_cache[package_name]
        if not package.is_installed:
            print "Installing system package " + package_name
            if not dry_run:
                package.mark_install()
                apt_cache.commit()
        else:
            print "Skipping package " + package_name + ", which is already installed"
        print "Done"
    except TypeError, te:
        print "Wrong types: " + str(e)
    except KeyError, ke:
        print "Unknown package " + package_name
    except apt.cache.LockFailedException, lfe:
        print "Failed to install " + package_name + ":", \
              "Can't lock the the package cache"

# install header and dynamic libraries for the live555 video stremer
def live(address, clean, dry_run, prefix = "/usr/local/"):
    print "Installing live555 in " + prefix
    dirs = ['BasicUsageEnvironment', 'UsageEnvironment', 'groupsock', 'liveMedia']

    if clean:
        execute('sudo rm -rf ' + ' '.join(dirs), prefix + 'include', dry_run)
        for d in dirs:
            execute('sudo rm -rf lib' + d + '*', prefix + 'lib/', dry_run)
        if os.path.isdir("/tmp/live555"):
            execute('make distclean', '/tmp/live555', dry_run)
        return

    # check for exemplary library
    if os.path.isfile(prefix + "lib/libBasicUsageEnvironment.so"):
        print "Live555 already installed"
        return

    execute('git clone --depth 1 ' + address, '/tmp/', dry_run)

    target = 'linux-with-shared-libraries'
    execute('./genMakefiles ' + target, '/tmp/live555', dry_run)

    # configure for install in /usr instead of /usr/local ??
    if prefix != "/usr/local":
        for dir in dirs:
            replace_regex_in_file_line('/tmp/live555/' + dir + '/Makefile',
                                       '/usr/local/', prefix, dry_run)

    execute('make -j4', '/tmp/live555', dry_run)

    for dir in dirs:
        print 'sudo make install', '/tmp/live555/' + dir
        execute('sudo make install', '/tmp/live555/' + dir, dry_run)

    print "--> Live555 installed"

if __name__ == '__main__':

    if os.geteuid() != 0:
        exit("Gotta run this as root")

    dry_run = False  # really do sth?
    clean = False # remove manually installed stuff?

    pre = '/usr/'
    for a in sys.argv[1:]:
        if a == '-dry':
            dry_run = True
        elif a == '-clean':
            clean = True
        elif a == '-live':
            only_live = True
        elif a == '-local':
            pre = '/usr/local/'

    requirements = {
        'libx264-dev' : apt_get,
        'https://github.com/hackeron/live555.git' : live
    }

    for req in requirements:
        requirements[req](req, clean, dry_run, prefix = pre)
