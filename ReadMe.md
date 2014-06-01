NYC Tax Calculator
==================
This Python program calculates the total U.S. taxes owed by a single (unmarried) 
resident of New York City. It is limited in its scope and functionality, and 
was written primarily for personal use. It is also far from perfect, and the 
numbers calculated should only be considered as a guide. However, generally 
speaking, your tax should not _exceed_ the amounts calculated by this program.

Federal rates (2014) are based on http://www.forbes.com/sites/kellyphillipserb/2013/10/31/irs-announces-2014-tax-brackets-standard-deduction-amounts-and-more/ and other sources.

New York State & City tax rates (2014) are based on http://www.tax.ny.gov/pit/file/tax_tables.htm

#### Some Resources

* For more on state & local tax federal income tax deductions, see: http://taxes.about.com/od/Deductions/a/State-Income-Tax-Deduction.htm
* A Guide to New York Personal Income Tax: http://taxes.about.com/od/statetaxes/a/New-York-income-taxes.htm
    * Standard Deductions: http://www.tax.ny.gov/pit/file/standard_deductions.htm
    * Instructions for Form IT-201 (New York State Resident Income Tax Return): http://www.tax.ny.gov/pdf/2013/inc/it201i_2013.pdf
    * Instructions for Form IT-272 (New York State Residents' Claim for College Tuition Credit or Itemized Deduction): http://www.tax.ny.gov/pdf/current_forms/it/it272i.pdf
        * The non-refundable tax credit is limited to 4% of $10,000, i.e. $400.
        * The maximum itemized deduction for college is $10,000.
* A Guide to New York City Income Tax: http://taxes.about.com/od/statetaxes/a/New-York-City-Income-Tax.htm
    * New York City Credits: http://www.tax.ny.gov/pit/credits/new_york_city_credits.htm
    * Instructions for Form NYC-210 (Claim for NYC School Tax Credit): http://www.tax.ny.gov/pdf/current_forms/it/nyc210i.pdf

###Usage

    usage: nyc_tax_calculator.py [-h] [-e EXEMPTIONS] [-d DEDUCTIONS] income

    Calculates total U.S. taxes owed by a single (unmarried) resident of NYC.

    positional arguments:
      income                Annual Gross Income

    optional arguments:
      -h, --help            show this help message and exit
      -e EXEMPTIONS, --exemptions EXEMPTIONS
                            Number of personal exemptions. The default is 1.
      -d DEDUCTIONS, --deductions DEDUCTIONS
                            Itemized deduction other than the standard deduction,
                            personal exemptions, and state/local/foreign tax
                            deductions.

### Sample Output
Sample output rendered by the program for a person with a gross annual 
income of $85,000, no additional itemized deducations, and claiming a 
single personal exemption:

    Adjusted Gross Income = $85,000.00

    New York State Tax Deductions:
        New York State Standard Deduction = $7,700.00
    New York State Tax Deductions = $7,700.00

    New York State Taxable Income = $77,300.00

    New York State Income Tax:
       for the slab from $0.00 to $8,200.00 = $328.00 (at 4.00% on $8,200.00)
       for the slab from $8,200.00 to $11,300.00 = $139.50 (at 4.50% on $3,100.00)
       for the slab from $11,300.00 to $13,350.00 = $107.62 (at 5.25% on $2,050.00)
       for the slab from $13,350.00 to $20,550.00 = $424.80 (at 5.90% on $7,200.00)
       for the slab from $20,550.00 to $77,150.00 = $3,650.70 (at 6.45% on $56,600.00)
       for the slab from $77,150.00 to $77,300.00 = $9.97 (at 6.65% on $150.00)
    New York State Income Tax = $4,660.60

    New York City Income Tax:
       for the slab from $0.00 to $12,000.00 = $348.84 (at 2.91% on $12,000.00)
       for the slab from $12,000.00 to $25,000.00 = $459.42 (at 3.53% on $13,000.00)
       for the slab from $25,000.00 to $50,000.00 = $897.75 (at 3.59% on $25,000.00)
       for the slab from $50,000.00 to $77,300.00 = $995.90 (at 3.65% on $27,300.00)
       minus the New York City School Tax Credit of $63
    New York City Income Tax = $2,638.91

    Total New York State & City Taxes = $7,299.51

    Federal Tax Deductions:
        State, Local & Foreign Tax Deduction = $7,299.51
        Additional Itemized Deductions = $0.00
        Personal Exemptions = $3,950.00  (1 x $3,950.00)
    Federal Tax Deductions = $11,249.51

    Federal Taxable Income = $73,750.49

    Federal Income Tax:
       for the slab from $0.00 to $9,075.00 = $907.50 (at 10.00% on $9,075.00)
       for the slab from $9,075.00 to $36,900.00 = $4,173.75 (at 15.00% on $27,825.00)
       for the slab from $36,900.00 to $73,750.49 = $9,212.62 (at 25.00% on $36,850.49)
    Federal Income Tax = $14,293.87

    Minimum Federal Income Tax: $8,372.00 (at 26% on $32,200.00)

    Federal Insurance Contributions Act (FICA) Tax:
        Social Security's Old-Age, Survivors, and Disability Insurance (OASDI) Tax: $5,270.00 (at 6.20% flat on $85,000.00)
        Medicare's Hospital Insurance (HI) Tax: $1,232.50 (at 1.45% flat on $85,000.00)
    Federal Insurance Contributions Act (FICA) Tax = $6,502.50

    Total Federal Taxes = $20,796.37

    Total Federal, State & Local Tax: $28,095.89

    Income after taxation = $56,904.11
    Effective Tax Rate = 33.05%
    Monthly Income = $4,742.01

### Todos

* Fix the [AMT](https://en.wikipedia.org/wiki/Alternative_minimum_tax) 
  phase-out calculation.
* Fix the personal exemptions phase-out calculation.
