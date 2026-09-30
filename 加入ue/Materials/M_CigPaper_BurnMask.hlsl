// ============================================================================
//  M_CigPaper — burn front for the cigarette paper
//  Paste into a Material "Custom" node. Output Type: CMOT Float 3
//  Returns: x = OpacityMask (1 = unburnt paper), y = char amount, z = glow amount
//
//  Custom node inputs (names must match):
//    WorldPos   <- Absolute World Position (Excluding Material Offsets)
//    BurnFront  <- VectorParameter "BurnFront"   (set every frame by ACigarette)
//    Axis       <- VectorParameter "CigAxis"     (filter -> tip, world space)
//    CharWidth  <- ScalarParameter "CharWidth"   default 0.35   (cm of blackened paper)
//    GlowWidth  <- ScalarParameter "GlowWidth"   default 0.12   (cm of glowing edge)
//    Time       <- Time
// ============================================================================

// signed distance along the cigarette from the burn front: <0 = still paper, >0 = already burnt
float d = dot(WorldPos - BurnFront, Axis);

// a ragged, slowly crawling edge instead of a perfectly straight cut
float3 p = WorldPos * 3.1;
float n1 = frac(sin(dot(floor(p.yz * 4.0), float2(12.9898, 78.233))) * 43758.5453);
float n2 = sin(p.y * 7.0 + Time * 0.6) * sin(p.z * 6.0 - Time * 0.4);
float ragged = (n1 - 0.5) * 0.06 + n2 * 0.03;
d += ragged;

float keep   = d < 0.0 ? 1.0 : 0.0;
float charA  = saturate(1.0 - (-d) / CharWidth);
float glowA  = saturate(1.0 - (-d) / GlowWidth);
glowA *= lerp(0.45, 1.0, frac(n1 * 7.13 + Time * 0.35));   // patchy, flowing embers

return float3(keep, charA * charA, glowA);


// ============================================================================
//  Wiring for M_CigPaper
//  Material: Blend Mode = Masked, Two Sided = on, Shading Model = Default Lit
//
//  BaseColor   = lerp( PaperColor(0.93,0.91,0.86) * PaperFibreTexture ,
//                      CharColor(0.03,0.02,0.015) , Custom.y )
//  Roughness   = lerp( 0.75 , 0.95 , Custom.y )
//  Normal      = paper fibre normal map (tiling along the length), strength 0.3
//  Emissive    = Custom.z * EmberColor(1.0, 0.32, 0.06) * Glow * 25 * Lit
//                (ScalarParameters "Glow" and "Lit" are driven by ACigarette)
//  OpacityMask = Custom.x
//  Optional: a faint seam line and a small printed logo near the filter via a mask texture.
//
//  M_CigEmber  (on the EmberCap disc)
//    Unlit or Default Lit with Emissive = EmberNoise(panning) * EmberColor * Glow * 30
//    BaseColor = near black (0.03), Roughness 1.
//    EmberNoise: a cloudy noise texture panning slowly (0.05 uv/s) — gives the "breathing" cherry.
//
//  M_CigAsh  (on the Ash cylinder)
//    BaseColor = 0.55 grey * ash ring texture (thin darker bands along the length)
//    Roughness = 1, Normal = crumbly noise normal, strength 1
//    Near the ember end (first ~3 mm) lerp towards 0.08 grey — freshly burnt ash is dark.
//    WPO droop: offset -Z by  pow(LocalX_fromFront / AshLength, 2) * 0.15cm  once the ash is > 1.5 cm.
// ============================================================================
