# Author: petter.strandmark@gmail.com (Petter Strandmark)

SET(MESCHACH_INTERNAL_SRC
  bdfactor.c
  bkpfacto.c
  chfactor.c
  copy.c
  err.c
  extras.c
  fft.c
  givens.c
  hessen.c
  hsehldr.c
  init.c
  iter0.c
  iternsym.c
  itersym.c
  ivecop.c
  lufactor.c
  machine.c
  matlab.c
  matop.c
  matrixio.c
  meminfo.c
  memory.c
  memstat.c
  mfunc.c
  norm.c
  otherio.c
  pxop.c
  qrfactor.c
  schur.c
  solve.c
  sparse.c
  sparseio.c
  spbkp.c
  spchfctr.c
  splufctr.c
  sprow.c
  spswap.c
  submat.c
  svd.c
  symmeig.c
  update.c
  vecop.c
  version.c
#  zcopy.c
#  zfunc.c
#  zgivens.c
#  zhessen.c
#  zhsehldr.c
#  zlufctr.c
#  zmachine.c
#  zmatio.c
#  zmatlab.c
#  zmatop.c
#  zmemory.c
#  znorm.c
#  zqrfctr.c
#  zschur.c
#  zsolve.c
#  ztorture.c
#  zvecop.c
)

FILE(GLOB MESCHACH_HEADERS *.h)

SET(MESCHACH_LIBRARY_SOURCE
    ${MESCHACH_INTERNAL_SRC}
    ${MESCHACH_HEADERS})

ADD_LIBRARY(meschach STATIC ${MESCHACH_LIBRARY_SOURCE})

INSTALL(TARGETS meschach
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

ADD_EXECUTABLE(test_meschach_torture torture.c)
    TARGET_LINK_LIBRARIES(test_meschach_torture meschach)
    IF (NOT MSVC)
      TARGET_LINK_LIBRARIES(test_meschach_torture m)
    ENDIF (NOT MSVC)

ADD_TEST(NAME meschach_torture
         COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_meschach_torture)
SET_PROPERTY(TARGET test_meschach_torture         PROPERTY FOLDER "Tests")

ADD_EXECUTABLE(test_meschach_sptort sptort.c)
    TARGET_LINK_LIBRARIES(test_meschach_sptort meschach)
    IF (NOT MSVC)
      TARGET_LINK_LIBRARIES(test_meschach_sptort m)
    ENDIF (NOT MSVC)

ADD_TEST(NAME meschach_sptort
         COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_meschach_sptort)
SET_PROPERTY(TARGET test_meschach_sptort         PROPERTY FOLDER "Tests")