FROM gitlab-registry.cern.ch/linuxsupport/alma9-base

RUN yum -y update && yum install https://linuxsoft.cern.ch/wlcg/el9/aarch64/wlcg-repo-1.0.0-1.el9.noarch.rpm -y \
    && yum -y install HEP_OSlibs glibc-langpack-en cmake make g++ uuid python3 libcap pkg-config valgrind unzip nano krb5-libs \
              expat-devel openldap-devel krb5-devel libnsl2-devel pam-devel keyutils-libs-devel lksctp-tools-devel which \
    && yum clean all

WORKDIR /RunDCSCalc
RUN mkdir logfiles

COPY run container/execute.sh ./
COPY build ./build

CMD ["./execute.sh"]