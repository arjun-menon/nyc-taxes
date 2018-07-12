type writeDetail = (string, string, option(float)) => unit;

let p = (n: float, percentage: float) : float => n *. percentage /. 100.0;

let calc_slab_tax = (income: float, schedule: TaxRates.incomeTaxRateSchedule) : float => {
  let prev_limit = ref(0.0);
  List.fold_left(
    (result: float, bracket: TaxRates.incomeBracket) => {
      let (limit: float, rate: float) = bracket;

      if (income <= prev_limit^) {
        result;
      } else {
        let slab_amt: float = income <= limit ? income -. prev_limit^ : limit -. prev_limit^;

        let tax_amt: float = p(slab_amt, rate);

        Js.log(tax_amt);
        /*slab_limit = income > limit ? ns(limit) : ns(income);*/
        /*w('b', 'Slab from ' + ns(prev_limit) + ' to ' + slab_limit + ' = ' + ns(tax_amt) + ' (at ' + rate + '% on ' + ns(slab_amt) + ')');*/

        prev_limit := limit;
        result +. tax_amt;
      };
    },
    0.0,
    schedule,
  );
};

let calc_taxes =
    (taxRates: TaxRates.taxRates, income: float, itemized_deductions: float, exemptions: int, w: writeDetail)
    : float => {
  w("a", "Adjusted Gross Income", Some(income));

  /* ------------- New York State & City ------------- */

  /*w('l_start', 'New York State Tax Deductions');*/

  let nys_basic_deduction: float =
    if (itemized_deductions <= taxRates.nyc.standardDeduction) {
      /*w('b', 'New York State Standard Deduction = ' + ns(taxRates.nyc.standardDeduction));*/
      taxRates.nyc.
        standardDeduction;
    } else {
      /*w('b', 'Itemized Deductions = ' + ns(itemized_deductions));*/
      itemized_deductions;
    };

  /* NYS Personal Exemptions for Dependents */
  let no_of_dependents: int = Pervasives.max(exemptions - 1, 0);

  let nys_personal_exemptions: float = float(no_of_dependents) *. taxRates.nyc.dependentPersonalExemption;

  /*if (no_of_dependents > 0)*/
  /*w('b', 'Personal exemptions for dependents = ' +*/
  /*ns(nys_personal_exemptions) + '  (' + no_of_dependents +*/
  /*' x ' + ns(taxRates.nyc.dependentPersonalExemption) + ')');*/

  let nys_deduction = nys_basic_deduction +. nys_personal_exemptions;

  /*w('l_end', 'New York State Tax Deductions', ns(nys_deduction));*/

  let nys_taxable_income = Pervasives.max(income -. nys_deduction, 0.0);

  /*w('a', 'New York State Taxable Income', ns(nys_taxable_income));*/

  /*w('l_start', 'New York City Income Tax');*/

  let nyc_income_tax = calc_slab_tax(nys_taxable_income, taxRates.nyc.cityRateSchedule);

  /*w('l_end', 'New York City Income Tax', ns(nyc_income_tax));*/

  /*w('l_start', 'New York State Income Tax');*/

  let nys_income_tax = calc_slab_tax(nys_taxable_income, taxRates.nyc.stateRateSchedule);

  /*w('l_end', 'New York State Income Tax', ns(nys_income_tax));*/

  let total_state_and_local_income_tax = nys_income_tax +. nyc_income_tax;

  /*w('a', 'Total New York State & City Taxes', ns(total_state_and_local_income_tax));*/

  /* ------------- Federal ------------- */

  /*w('l_start', 'Federal Tax Deductions');*/

  let federal_itemized_deductions = itemized_deductions +. total_state_and_local_income_tax;

  let federal_basic_deduction =
    if (federal_itemized_deductions > taxRates.federal.income.standardDeduction) {
      /*w('b', 'State and Local Taxes Deduction = ' + ns(total_state_and_local_income_tax));*/
      /*w('b', 'Additional Itemized Deductions = ' + ns(itemized_deductions));*/
      federal_itemized_deductions;
    } else {
      /*w('b', 'Federal Standard Deduction = ' + ns(federal_deduction));*/
      taxRates.federal.income.
        standardDeduction;
    };

  let federal_personal_exemptions = float(exemptions) *. taxRates.federal.income.personalExemption;

  /*w('b', 'Personal Exemptions = ' + ns(federal_personal_exemptions) +*/
  /*'  (' + exemptions + ' x ' + ns(taxRates.federal.personal_exemption) + ')');*/

  let federal_deduction = federal_basic_deduction +. federal_personal_exemptions;

  /*w('l_end', 'Federal Tax Deductions', ns(federal_deduction));*/

  let federal_taxable_income: float = Pervasives.max(income -. federal_deduction, 0.0);

  /*w('a', 'Federal Taxable Income', ns(federal_taxable_income));*/

  /*w('l_start', 'Federal Income Tax');*/

  let federal_income_tax = calc_slab_tax(federal_taxable_income, taxRates.federal.income.rateSchedule);

  /*w('l_end', 'Federal Income Tax', ns(federal_income_tax));*/

  /*w('l_start', 'Federal Insurance Contributions Act (FICA) Tax');*/

  let social_security_taxable_income: float = Pervasives.min(income, taxRates.federal.fica.socialSecurityWageBase);
  let social_security_tax: float = p(social_security_taxable_income, taxRates.federal.fica.socialSecurityTaxRate);

  /*w('b', "Social Security's Old-Age, Survivors, and Disability Insurance (OASDI) Tax: " +*/
  /*ns(social_security_tax) + ' (at ' + taxRates.federal.social_security_tax_rate +*/
  /*'% flat on ' + ns(social_security_taxable_income) + ')');*/

  let medicare_tax: float = p(income, taxRates.federal.fica.medicareTaxRate);

  /*w('b', "Medicare's Hospital Insurance (HI) Tax: " + ns(medicare_tax) +*/
  /*' (at ' + taxRates.federal.medicare_tax_rate + '% flat on ' + ns(income) + ')');*/

  let fica_tax = social_security_tax +. medicare_tax;

  /*w('l_end', 'Federal Insurance Contributions Act (FICA) Tax', ns(fica_tax));*/

  let total_federal_tax = federal_income_tax +. fica_tax;

  /*w('a', 'Total Federal Taxes', ns(total_federal_tax));*/

  /* ----------------------- Total ----------------------- */

  let total_tax = total_federal_tax +. total_state_and_local_income_tax;

  /*w('a', 'Total Federal, State & Local Taxes', ns(total_tax));*/

  /*w('x', '<hr/>');*/

  /*income_after_tax = income - total_tax;*/

  /*monthly_income_after_tax = income_after_tax / 12.0;*/

  /*w('a', 'Income after Taxation', ns(income_after_tax));*/

  /*w('a', 'Effective Tax Rate', (total_tax * 100.0 / income).toFixed(2) + '%');*/

  /*w('a', 'Monthly Income', ns(monthly_income_after_tax));*/

  /*if (itemized_deductions > 0) {*/
  /*w('x', '<hr/>');*/

  /*income_after_taxes_and_deductions = income_after_tax - itemized_deductions;*/

  /*monthly_income_after_taxes_and_deductions = income_after_taxes_and_deductions / 12.0;*/

  /*w('a', 'Income after Taxes & Deductible Spending', ns(income_after_taxes_and_deductions));*/

  /*w('a', 'Monthly Income after Taxes & Deductible Spending', ns(monthly_income_after_taxes_and_deductions));*/
  /*}*/

  total_tax;
};
