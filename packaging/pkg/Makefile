# $FreeBSD$

PORTNAME=	pci
PORTVERSION=	0.1.0
CATEGORIES=	sysutils

MAINTAINER=	chuck@tuffli.net
COMMENT=	Diagnostic utility for the PCI bus

LICENSE=	BSD
LICENSE_FILE=	${WRKSRC}/LICENSE

LIB_DEPENDS=	libpciaccess.so:devel/libpciaccess

CONFIGURE_ARGS+= --mandir="${MANPREFIX}/man"
GNU_CONFIGURE=	yes
USES=		autoreconf gmake

CPPFLAGS+=	-I${LOCALBASE}/include
LIBS+=		-L${LOCALBASE}/lib

PLIST_FILES=	bin/pci \
		man/man8/pci.8

USE_GITHUB=	yes
GH_TUPLE=	ctuffli:pci:v0.1.0

pre-configure:
	@(cd ${CONFIGURE_WRKSRC} && ./autogen.sh)

.include <bsd.port.mk>
