//===-- llvm/MC/MCObjectFileInfo.h - Object File Info -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes common object file formats.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCOBJECTFILEINFO_H
#define LLVM_MC_MCOBJECTFILEINFO_H

#include "llvm/ADT/Triple.h"
#include "llvm/Support/CodeGen.h"

namespace llvm {
class MCContext;
class MCSection;

class MCObjectFileInfo {
protected:
  /// True if .comm supports alignment.  This is a hack for as long as we
  /// support 10.4 Tiger, whose assembler doesn't support alignment on comm.
  bool CommDirectiveSupportsAlignment;

  /// True if target object file supports a weak_definition of constant 0 for an
  /// omitted EH frame.
  bool SupportsWeakOmittedEHFrame;

  /// True if the target object file supports emitting a compact unwind section
  /// without an associated EH frame section.
  bool SupportsCompactUnwindWithoutEHFrame;

  /// Some encoding values for EH.
  unsigned PersonalityEncoding;
  unsigned LSDAEncoding;
  unsigned FDECFIEncoding;
  unsigned TTypeEncoding;

  /// Section flags for eh_frame
  unsigned EHSectionType;
  unsigned EHSectionFlags;

  /// Compact unwind encoding indicating that we should emit only an EH frame.
  unsigned CompactUnwindDwarfEHFrameOnly;

  /// Section directive for standard text.
  MCSection *TextSection;

  /// Section directive for standard data.
  MCSection *DataSection;

  /// Section that is default initialized to zero.
  MCSection *BSSSection;

  /// Section that is readonly and can contain arbitrary initialized data.
  /// Targets are not required to have a readonly section. If they don't,
  /// various bits of code will fall back to using the data section for
  /// constants.
  MCSection *ReadOnlySection;

  /// This section contains the static constructor pointer list.
  MCSection *StaticCtorSection;

  /// This section contains the static destructor pointer list.
  MCSection *StaticDtorSection;

  /// If exception handling is supported by the target, this is the section the
  /// Language Specific Data Area information is emitted to.
  MCSection *LSDASection;

  /// If exception handling is supported by the target and the target can
  /// support a compact representation of the CIE and FDE, this is the section
  /// to emit them into.
  MCSection *CompactUnwindSection;

  // Dwarf sections for debug info.  If a target supports debug info, these must
  // be set.
  MCSection *DwarfAbbrevSection;
  MCSection *DwarfInfoSection;
  MCSection *DwarfLineSection;
  MCSection *DwarfFrameSection;
  MCSection *DwarfPubTypesSection;
  const MCSection *DwarfDebugInlineSection;
  MCSection *DwarfStrSection;
  MCSection *DwarfLocSection;
  MCSection *DwarfARangesSection;
  MCSection *DwarfRangesSection;
  // The pubnames section is no longer generated by default.  The generation
  // can be enabled by a compiler flag.
  MCSection *DwarfPubNamesSection;

  /// DWARF5 Experimental Debug Info Sections
  /// DwarfAccelNamesSection, DwarfAccelObjCSection,
  /// DwarfAccelNamespaceSection, DwarfAccelTypesSection -
  /// If we use the DWARF accelerated hash tables then we want to emit these
  /// sections.
  MCSection *DwarfAccelNamesSection;
  MCSection *DwarfAccelObjCSection;
  MCSection *DwarfAccelNamespaceSection;
  MCSection *DwarfAccelTypesSection;

  // These are used for the Fission separate debug information files.
  MCSection *DwarfInfoDWOSection;
  MCSection *DwarfTypesDWOSection;
  MCSection *DwarfAbbrevDWOSection;
  MCSection *DwarfStrDWOSection;
  MCSection *DwarfLineDWOSection;
  MCSection *DwarfLocDWOSection;
  MCSection *DwarfStrOffDWOSection;
  MCSection *DwarfAddrSection;

  /// Section for newer gnu pubnames.
  MCSection *DwarfGnuPubNamesSection;
  /// Section for newer gnu pubtypes.
  MCSection *DwarfGnuPubTypesSection;

  MCSection *COFFDebugSymbolsSection;

  /// Extra TLS Variable Data section.
  ///
  /// If the target needs to put additional information for a TLS variable,
  /// it'll go here.
  MCSection *TLSExtraDataSection;

  /// Section directive for Thread Local data. ELF, MachO and COFF.
  MCSection *TLSDataSection; // Defaults to ".tdata".

  /// Section directive for Thread Local uninitialized data.
  ///
  /// Null if this target doesn't support a BSS section. ELF and MachO only.
  MCSection *TLSBSSSection; // Defaults to ".tbss".

  /// StackMap section.
  MCSection *StackMapSection;

  /// FaultMap section.
  MCSection *FaultMapSection;

  /// EH frame section.
  ///
  /// It is initialized on demand so it can be overwritten (with uniquing).
  MCSection *EHFrameSection;

  // ELF specific sections.
  MCSection *DataRelSection;
  const MCSection *DataRelLocalSection;
  MCSection *DataRelROSection;
  MCSection *DataRelROLocalSection;
  MCSection *MergeableConst4Section;
  MCSection *MergeableConst8Section;
  MCSection *MergeableConst16Section;

  // MachO specific sections.

  /// Section for thread local structure information.
  ///
  /// Contains the source code name of the variable, visibility and a pointer to
  /// the initial value (.tdata or .tbss).
  MCSection *TLSTLVSection; // Defaults to ".tlv".

  /// Section for thread local data initialization functions.
  const MCSection *TLSThreadInitSection; // Defaults to ".thread_init_func".

  MCSection *CStringSection;
  MCSection *UStringSection;
  MCSection *TextCoalSection;
  MCSection *ConstTextCoalSection;
  MCSection *ConstDataSection;
  MCSection *DataCoalSection;
  MCSection *DataCommonSection;
  MCSection *DataBSSSection;
  MCSection *FourByteConstantSection;
  MCSection *EightByteConstantSection;
  MCSection *SixteenByteConstantSection;
  MCSection *LazySymbolPointerSection;
  MCSection *NonLazySymbolPointerSection;

  /// COFF specific sections.
  MCSection *DrectveSection;
  MCSection *PDataSection;
  MCSection *XDataSection;
  MCSection *SXDataSection;

public:
  void InitMCObjectFileInfo(const Triple &TT, Reloc::Model RM,
                            CodeModel::Model CM, MCContext &ctx);
  LLVM_ATTRIBUTE_DEPRECATED(
      void InitMCObjectFileInfo(StringRef TT, Reloc::Model RM,
                                CodeModel::Model CM, MCContext &ctx),
      "StringRef GNU Triple argument replaced by a llvm::Triple object");

  bool getSupportsWeakOmittedEHFrame() const {
    return SupportsWeakOmittedEHFrame;
  }
  bool getSupportsCompactUnwindWithoutEHFrame() const {
    return SupportsCompactUnwindWithoutEHFrame;
  }
  bool getCommDirectiveSupportsAlignment() const {
    return CommDirectiveSupportsAlignment;
  }

  unsigned getPersonalityEncoding() const { return PersonalityEncoding; }
  unsigned getLSDAEncoding() const { return LSDAEncoding; }
  unsigned getFDEEncoding() const { return FDECFIEncoding; }
  unsigned getTTypeEncoding() const { return TTypeEncoding; }

  unsigned getCompactUnwindDwarfEHFrameOnly() const {
    return CompactUnwindDwarfEHFrameOnly;
  }

  MCSection *getTextSection() const { return TextSection; }
  MCSection *getDataSection() const { return DataSection; }
  MCSection *getBSSSection() const { return BSSSection; }
  MCSection *getLSDASection() const { return LSDASection; }
  MCSection *getCompactUnwindSection() const { return CompactUnwindSection; }
  MCSection *getDwarfAbbrevSection() const { return DwarfAbbrevSection; }
  MCSection *getDwarfInfoSection() const { return DwarfInfoSection; }
  MCSection *getDwarfLineSection() const { return DwarfLineSection; }
  MCSection *getDwarfFrameSection() const { return DwarfFrameSection; }
  MCSection *getDwarfPubNamesSection() const { return DwarfPubNamesSection; }
  MCSection *getDwarfPubTypesSection() const { return DwarfPubTypesSection; }
  MCSection *getDwarfGnuPubNamesSection() const {
    return DwarfGnuPubNamesSection;
  }
  MCSection *getDwarfGnuPubTypesSection() const {
    return DwarfGnuPubTypesSection;
  }
  const MCSection *getDwarfDebugInlineSection() const {
    return DwarfDebugInlineSection;
  }
  MCSection *getDwarfStrSection() const { return DwarfStrSection; }
  MCSection *getDwarfLocSection() const { return DwarfLocSection; }
  MCSection *getDwarfARangesSection() const { return DwarfARangesSection; }
  MCSection *getDwarfRangesSection() const { return DwarfRangesSection; }

  // DWARF5 Experimental Debug Info Sections
  MCSection *getDwarfAccelNamesSection() const {
    return DwarfAccelNamesSection;
  }
  MCSection *getDwarfAccelObjCSection() const { return DwarfAccelObjCSection; }
  MCSection *getDwarfAccelNamespaceSection() const {
    return DwarfAccelNamespaceSection;
  }
  MCSection *getDwarfAccelTypesSection() const {
    return DwarfAccelTypesSection;
  }
  MCSection *getDwarfInfoDWOSection() const { return DwarfInfoDWOSection; }
  MCSection *getDwarfTypesSection(uint64_t Hash) const;
  MCSection *getDwarfTypesDWOSection() const { return DwarfTypesDWOSection; }
  MCSection *getDwarfAbbrevDWOSection() const { return DwarfAbbrevDWOSection; }
  MCSection *getDwarfStrDWOSection() const { return DwarfStrDWOSection; }
  MCSection *getDwarfLineDWOSection() const { return DwarfLineDWOSection; }
  MCSection *getDwarfLocDWOSection() const { return DwarfLocDWOSection; }
  MCSection *getDwarfStrOffDWOSection() const { return DwarfStrOffDWOSection; }
  MCSection *getDwarfAddrSection() const { return DwarfAddrSection; }

  MCSection *getCOFFDebugSymbolsSection() const {
    return COFFDebugSymbolsSection;
  }

  MCSection *getTLSExtraDataSection() const { return TLSExtraDataSection; }
  const MCSection *getTLSDataSection() const { return TLSDataSection; }
  MCSection *getTLSBSSSection() const { return TLSBSSSection; }

  MCSection *getStackMapSection() const { return StackMapSection; }
  MCSection *getFaultMapSection() const { return FaultMapSection; }

  // ELF specific sections.
  MCSection *getDataRelSection() const { return DataRelSection; }
  const MCSection *getDataRelLocalSection() const {
    return DataRelLocalSection;
  }
  MCSection *getDataRelROSection() const { return DataRelROSection; }
  MCSection *getDataRelROLocalSection() const { return DataRelROLocalSection; }
  const MCSection *getMergeableConst4Section() const {
    return MergeableConst4Section;
  }
  const MCSection *getMergeableConst8Section() const {
    return MergeableConst8Section;
  }
  const MCSection *getMergeableConst16Section() const {
    return MergeableConst16Section;
  }

  // MachO specific sections.
  const MCSection *getTLSTLVSection() const { return TLSTLVSection; }
  const MCSection *getTLSThreadInitSection() const {
    return TLSThreadInitSection;
  }
  const MCSection *getCStringSection() const { return CStringSection; }
  const MCSection *getUStringSection() const { return UStringSection; }
  MCSection *getTextCoalSection() const { return TextCoalSection; }
  const MCSection *getConstTextCoalSection() const {
    return ConstTextCoalSection;
  }
  const MCSection *getConstDataSection() const { return ConstDataSection; }
  const MCSection *getDataCoalSection() const { return DataCoalSection; }
  const MCSection *getDataCommonSection() const { return DataCommonSection; }
  MCSection *getDataBSSSection() const { return DataBSSSection; }
  const MCSection *getFourByteConstantSection() const {
    return FourByteConstantSection;
  }
  const MCSection *getEightByteConstantSection() const {
    return EightByteConstantSection;
  }
  const MCSection *getSixteenByteConstantSection() const {
    return SixteenByteConstantSection;
  }
  MCSection *getLazySymbolPointerSection() const {
    return LazySymbolPointerSection;
  }
  MCSection *getNonLazySymbolPointerSection() const {
    return NonLazySymbolPointerSection;
  }

  // COFF specific sections.
  MCSection *getDrectveSection() const { return DrectveSection; }
  MCSection *getPDataSection() const { return PDataSection; }
  MCSection *getXDataSection() const { return XDataSection; }
  MCSection *getSXDataSection() const { return SXDataSection; }

  MCSection *getEHFrameSection() {
    if (!EHFrameSection)
      InitEHFrameSection();
    return EHFrameSection;
  }

  enum Environment { IsMachO, IsELF, IsCOFF };
  Environment getObjectFileType() const { return Env; }

  Reloc::Model getRelocM() const { return RelocM; }

private:
  Environment Env;
  Reloc::Model RelocM;
  CodeModel::Model CMModel;
  MCContext *Ctx;
  Triple TT;

  void initMachOMCObjectFileInfo(Triple T);
  void initELFMCObjectFileInfo(Triple T);
  void initCOFFMCObjectFileInfo(Triple T);

  /// Initialize EHFrameSection on demand.
  void InitEHFrameSection();

public:
  const Triple &getTargetTriple() const { return TT; }
};

} // end namespace llvm

#endif
