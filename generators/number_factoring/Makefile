all: nftocnf

include $(SMAKE)

nftocnf: 
	ghc -o nftocnf nftocnf.hs -XPatternSignatures -XMultiParamTypeClasses -XFunctionalDependencies -XFlexibleInstances
	mv nftocnf ../../../bin
	cp genFactorProblems.py ../../../bin


