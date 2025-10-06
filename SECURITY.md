# Security Policy

## Responsible Use

SSAPT is designed for testing and educational purposes only. Users must:

1. Only use SSAPT in authorized testing environments
2. Obtain proper authorization before deploying in production
3. Comply with all applicable laws and regulations
4. Respect terms of service of third-party applications
5. Not use SSAPT for malicious purposes

## Known Limitations

### Bypass Methods

SSAPT can be bypassed through several methods:

1. **Alternative APIs**
   - Windows.Graphics.Capture API (not currently hooked)
   - DXGI Desktop Duplication API
   - Windows Media Foundation

2. **Hardware Capture**
   - External capture cards
   - HDMI capture devices
   - Phone cameras

3. **Driver Manipulation**
   - DLL unloading/ejection
   - Process termination
   - Memory manipulation

4. **Kernel-Level Capture**
   - Kernel-mode screen capture
   - Display driver manipulation

### Application-Level Limitations

- Only works within the same process
- Cannot protect against kernel-mode capture
- May conflict with overlays (Discord, OBS, etc.)
- Performance impact on graphics-intensive applications

## Vulnerability Reporting

If you discover a security vulnerability in SSAPT:

1. **Do NOT** publicly disclose the vulnerability
2. Email details to the maintainers (if public repo, create a private security advisory)
3. Include:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if available)

4. Allow reasonable time for a fix before public disclosure

## Security Best Practices

### For Developers Using SSAPT

1. **Defense in Depth**
   - Don't rely solely on SSAPT for protection
   - Implement multiple layers of security
   - Monitor for bypass attempts

2. **Regular Updates**
   - Keep SSAPT up to date
   - Monitor for new bypass methods
   - Test regularly

3. **User Education**
   - Inform users about limitations
   - Provide clear security expectations
   - Document protection scope

4. **Monitoring**
   - Log screenshot attempts
   - Detect unusual behavior
   - Alert on potential bypass attempts

### For Users

1. **Verify Integrity**
   - Download from trusted sources only
   - Verify file hashes
   - Check digital signatures

2. **Understand Limitations**
   - Know what SSAPT can and cannot protect
   - Don't assume complete protection
   - Use additional security measures

3. **Keep Updated**
   - Update to latest version
   - Review security advisories
   - Apply patches promptly

## Threat Model

### In Scope

- Screenshot capture via GDI APIs
- DirectX frame buffer access
- Common screenshot tools and utilities
- Application-level capture methods

### Out of Scope

- Kernel-mode capture
- Hardware-based capture
- Physical photography
- Advanced persistent threats (APTs)
- State-level actors

## Compliance

### Legal Considerations

Users must ensure compliance with:

- Local laws and regulations
- Terms of service of third-party applications
- Industry-specific regulations (e.g., HIPAA, GDPR)
- Corporate security policies

### Ethical Guidelines

- Use only for authorized testing
- Respect privacy and consent
- Consider impact on accessibility
- Document and disclose limitations

## Incident Response

If you suspect SSAPT has been compromised or bypassed:

1. **Immediate Actions**
   - Stop using affected systems
   - Document the incident
   - Preserve evidence

2. **Assessment**
   - Determine scope of compromise
   - Identify affected data/systems
   - Assess potential impact

3. **Remediation**
   - Update to latest version
   - Implement additional controls
   - Review security posture

4. **Communication**
   - Notify stakeholders
   - Report to maintainers
   - Consider public disclosure (if applicable)

## Updates and Patches

Security updates will be released:

- As vulnerabilities are discovered and fixed
- Following responsible disclosure timelines
- With clear change logs and security advisories

## Contact

For security concerns:
- Create a private security advisory (GitHub)
- Email maintainers directly
- Use PGP encryption for sensitive information

## Acknowledgments

We appreciate responsible disclosure and will acknowledge security researchers who report vulnerabilities in accordance with this policy.
