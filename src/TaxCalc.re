let p = (n: float, percentage: float): float => n *. percentage /. 100.0;

type slabItem = {
  fromAmt: float,
  toAmt: float,
  taxAmt: float,
  taxRate: float,
  slabAmt: float,
};

let calcSlabTax = (income: float, schedule: TaxRates.incomeTaxRateSchedule): (float, array(slabItem)) => {
  let prev_limit = ref(0.0);
  let (totalTax, taxSlabs) =
    List.fold_left(
      (result: (float, list(slabItem)), bracket: TaxRates.incomeBracket) => {
        let (limit: float, rate: float) = bracket;

        if (income <= prev_limit^) {
          result;
        } else {
          let (tax_so_far, slabs) = result;
          let slab_amt: float = income <= limit ? income -. prev_limit^ : limit -. prev_limit^;
          let tax_amt: float = p(slab_amt, rate);
          let slab_limit = income > limit ? limit : income;

          let slab: slabItem = {
            fromAmt: prev_limit^,
            toAmt: slab_limit,
            taxAmt: tax_amt,
            taxRate: rate,
            slabAmt: slab_amt,
          };

          prev_limit := limit;
          let accumulated_tax = tax_so_far +. tax_amt;

          (accumulated_tax, [slab, ...slabs]);
        };
      },
      (0.0, []),
      schedule,
    );

  (totalTax, Array.of_list(List.rev(taxSlabs)));
};

type stateDeductionType =
  | StateStandardDeduction
  | StateItemizedDeductions;

type personalExemptionType = {
  oneExemptionAmt: float,
  numOfExemptions: int,
  totalExemptionsAmt: float,
};

type federalItemizedDeductions = {
  stateAndLocalTaxesDeduction: float,
  otherItemizedDeductions: float,
  stateTaxableIncomeReductions: float,
};

type federalDeductionType =
  | FederalStandardDeduction(float)
  | FederalItemizedDeductions(federalItemizedDeductions);

type federalPersonalExemptionsType =
  | None
  | Some(personalExemptionType);

type taxesAnalysis = {
  income: float,
  stateDeduction: stateDeductionType,
  stateDeductionAmt: float,
  statePersonalExemptions: personalExemptionType,
  stateTaxableIncomeReductions: float,
  stateTaxableIncome: float,
  cityIncomeTax: float,
  cityIncomeTaxSlabs: array(slabItem),
  stateIncomeTax: float,
  stateIncomeTaxSlabs: array(slabItem),
  totalStateAndLocalIncomeTax: float,
  federalDeduction: federalDeductionType,
  federalPersonalExemptions: federalPersonalExemptionsType,
  federalTaxableIncomeReductions: float,
  federalTaxableIncome: float,
  federalIncomeTaxSlabs: array(slabItem),
  federalIncomeTax: float,
  socialSecurityTaxableIncome: float,
  socialSecurityTaxRate: float,
  socialSecurityTax: float,
  medicareTaxRate: float,
  medicareTax: float,
  ficaTax: float,
  totalFederalTax: float,
  totalTax: float,
  incomeAfterTax: float,
  incomeAfterTaxMonthly: float,
  effectiveTaxRate: float,
};

let calcTaxes =
    (~taxRates: TaxRates.taxRates, ~income: float, ~itemizedDeductions: float, ~exemptions: int): taxesAnalysis => {
  /* ------------- New York State & City Taxes ------------- */

  let (stateDeductionAmt: float, stateDeduction: stateDeductionType) =
    itemizedDeductions <= taxRates.nyc.standardDeduction
      ? (taxRates.nyc.standardDeduction, StateStandardDeduction) : (itemizedDeductions, StateItemizedDeductions);

  /* NYS Personal Exemptions for Dependents */
  let numOfExemptions: int = Pervasives.max(exemptions - 1, 0);
  let statePersonalExemptions: personalExemptionType = {
    oneExemptionAmt: taxRates.nyc.dependentPersonalExemption,
    numOfExemptions,
    totalExemptionsAmt: float(numOfExemptions) *. taxRates.nyc.dependentPersonalExemption,
  };

  let stateTaxableIncomeReductions = stateDeductionAmt +. statePersonalExemptions.totalExemptionsAmt;
  let stateTaxableIncome = Pervasives.max(income -. stateTaxableIncomeReductions, 0.0);
  let (cityIncomeTax, cityIncomeTaxSlabs) = calcSlabTax(stateTaxableIncome, taxRates.nyc.cityRateSchedule);
  let (stateIncomeTax, stateIncomeTaxSlabs) = calcSlabTax(stateTaxableIncome, taxRates.nyc.stateRateSchedule);
  let totalStateAndLocalIncomeTax = stateIncomeTax +. cityIncomeTax;

  /* ------------- Federal Taxes ------------- */
  let stateAndLocalTaxesDeduction =
    taxRates.federal.income.personalExemption === 0.0
      ? Pervasives.min(totalStateAndLocalIncomeTax, 10000.0) : totalStateAndLocalIncomeTax;
  let federalItemizedDeductions = itemizedDeductions +. stateAndLocalTaxesDeduction;
  let (federalDeductionAmt: float, federalDeduction: federalDeductionType) =
    federalItemizedDeductions <= taxRates.federal.income.standardDeduction
      ? (
        taxRates.federal.income.standardDeduction,
        FederalStandardDeduction(taxRates.federal.income.standardDeduction),
      )
      : (
        federalItemizedDeductions,
        FederalItemizedDeductions({
          stateAndLocalTaxesDeduction,
          otherItemizedDeductions: itemizedDeductions,
          stateTaxableIncomeReductions: federalItemizedDeductions,
        }),
      );
  let personalExemptions = {
    oneExemptionAmt: taxRates.federal.income.personalExemption,
    numOfExemptions: exemptions,
    totalExemptionsAmt: float(exemptions) *. taxRates.federal.income.personalExemption,
  };
  let federalPersonalExemptions = taxRates.federal.income.personalExemption > 0.0 ? Some(personalExemptions) : None;
  let federalTaxableIncomeReductions = federalDeductionAmt +. personalExemptions.totalExemptionsAmt;
  let federalTaxableIncome: float = Pervasives.max(income -. federalTaxableIncomeReductions, 0.0);
  let (federalIncomeTax, federalIncomeTaxSlabs) =
    calcSlabTax(federalTaxableIncome, taxRates.federal.income.rateSchedule);

  /* FICA (Social Security and Medicare) Taxes */
  let socialSecurityTaxableIncome: float = Pervasives.min(income, taxRates.federal.fica.socialSecurityWageBase);
  let socialSecurityTaxRate = taxRates.federal.fica.socialSecurityTaxRate;
  let socialSecurityTax: float = p(socialSecurityTaxableIncome, socialSecurityTaxRate);
  let medicareTaxRate = taxRates.federal.fica.medicareTaxRate;
  let medicareTax: float = p(income, medicareTaxRate);
  let ficaTax = socialSecurityTax +. medicareTax;

  let totalFederalTax = federalIncomeTax +. ficaTax;
  let totalTax: float = totalStateAndLocalIncomeTax +. totalFederalTax;
  let incomeAfterTax = income -. totalTax;
  let incomeAfterTaxMonthly = incomeAfterTax /. 12.0;
  let effectiveTaxRate = totalTax *. 100.0 /. income;

  {
    income,
    stateDeduction,
    stateDeductionAmt,
    statePersonalExemptions,
    stateTaxableIncomeReductions,
    stateTaxableIncome,
    cityIncomeTax,
    cityIncomeTaxSlabs,
    stateIncomeTax,
    stateIncomeTaxSlabs,
    totalStateAndLocalIncomeTax,
    federalDeduction,
    federalPersonalExemptions,
    federalTaxableIncomeReductions,
    federalTaxableIncome,
    federalIncomeTax,
    federalIncomeTaxSlabs,
    socialSecurityTaxableIncome,
    socialSecurityTaxRate,
    socialSecurityTax,
    medicareTaxRate,
    medicareTax,
    ficaTax,
    totalFederalTax,
    totalTax,
    incomeAfterTax,
    incomeAfterTaxMonthly,
    effectiveTaxRate,
  };
};