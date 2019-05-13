let rs = ReasonReact.string;

let parseint: string => int = [%raw {| x => {
  const v = parseInt(x, 10);
  return isNaN(v) ? 0 : v;
} |}];

let getParamsFromStrings = (year: string, income: string, deductions: string, exemptions: string): TaxCalc.taxParams => {
  year: parseint(year),
  income: Js.Float.fromString(income),
  deductions: Js.Float.fromString(deductions),
  exemptions: parseint(exemptions),
};

let checkYear = (year: int): bool => {
  switch (Belt.Array.getBy(TaxRates.availableYears, aYear => aYear == year)) {
  | Some(_) => true
  | None => false
  };
};

// any comaprison against NaN _floats_ yields false
let checkIncome = (income: float): bool => income > 0.0;
let checkDeductions = (deductions: float): bool => deductions >= 0.0;

let checkExemptions = (exemptions: int): bool => exemptions >= 0;

let checkParams = (params: TaxCalc.taxParams): bool =>
  checkYear(params.year)
  && checkIncome(params.income)
  && checkDeductions(params.deductions)
  && checkExemptions(params.exemptions);

let getParameterByNameRaw: (string, string) => string = [%bs.raw
  {|
  function getParameterByName(query, name) {
    // based on: http://stackoverflow.com/a/901144/908430
    name = name.replace(/[\[]/, '\\[').replace(/[\]]/, '\\]');
    const regex = new RegExp('[\\?&]' + name + '=([^&#]*)');
    const results = regex.exec('?' + query);
    return results == null ? '' : results[1].replace(/\+/g, ' ');
  }
|}
];

let getParameterByName = (query: string, name: string) =>
  Js_global.decodeURIComponent(getParameterByNameRaw(query, name));

let removeCommas: string => string = [%bs.raw
  {|
  function removeCommas(text) {
    return text.replace(/,/g, '');
  }
|}
];

let parseFloat = (s: string): float => Js.Float.fromString(removeCommas(s));

let makeUrlParams = (params: TaxCalc.taxParams): string => {
  let year = string_of_int(params.year);
  let income = Js.Float.toString(params.income);
  let deductions = Js.Float.toString(params.deductions);
  let exemptions = string_of_int(params.exemptions);
  "?year="
  ++ Js_global.encodeURIComponent(year)
  ++ "&income="
  ++ Js_global.encodeURIComponent(income)
  ++ "&deductions="
  ++ Js_global.encodeURIComponent(deductions)
  ++ "&exemptions="
  ++ Js_global.encodeURIComponent(exemptions);
};

type urlParams = {
  year: string,
  income: string,
  deductions: string,
  exemptions: string,
};

let getUrlParams = (url: ReasonReactRouter.url): urlParams => {
  let urlParams: string = url.search;
  {
    year: getParameterByName(urlParams, "year"),
    income: getParameterByName(urlParams, "income"),
    deductions: getParameterByName(urlParams, "deductions"),
    exemptions: getParameterByName(urlParams, "exemptions"),
  };
};
let destringUrlParams = (urlParams: urlParams): TaxCalc.taxParams =>
  getParamsFromStrings(urlParams.year, urlParams.income, urlParams.deductions, urlParams.exemptions);

let fixUrlIfNecessary = (defaultParams: TaxCalc.taxParams) => {
  let url = ReasonReactRouter.dangerouslyGetInitialUrl();
  if (url.search === "") {
    ReasonReactRouter.replace(makeUrlParams(defaultParams));
  } else {
    let params = ref(destringUrlParams(getUrlParams(url)));
    if (!checkYear(params^.year)) {
      params := {...params^, year: defaultParams.year};
    };
    if (!checkIncome(params^.income)) {
      params := {...params^, income: defaultParams.income};
    };
    if (!checkDeductions(params^.deductions)) {
      params := {...params^, deductions: defaultParams.deductions};
    };
    if (!checkExemptions(params^.exemptions)) {
      params := {...params^, exemptions: defaultParams.exemptions};
    };
    ReasonReactRouter.replace(makeUrlParams(params^));
  };
};

let eventS = (setValue, event) => {
  let value: string = ReactEvent.Form.target(event)##value;
  setValue(_ => value);
};

module TaxAnalyzer = {
  [@react.component]
  let make = () => {
    let urlParams = getUrlParams(ReasonReactRouter.useUrl());

    let params: TaxCalc.taxParams = destringUrlParams(urlParams);

    let (yearS, setYearS) = React.useState(() => urlParams.year);
    let (incomeS, setIncomeS) = React.useState(() => urlParams.income);
    let (deductionsS, setDeductionsS) = React.useState(() => urlParams.deductions);
    let (exemptionsS, setExemptionsS) = React.useState(() => urlParams.exemptions);

    let formParams: TaxCalc.taxParams = getParamsFromStrings(yearS, incomeS, deductionsS, exemptionsS);

    let onCalc = _ => {
      ReasonReactRouter.push(makeUrlParams(formParams));
    };

    let isCalcEnabled: bool = checkParams(formParams);

    let checkSign = (valid: bool) => {
      let color = valid ? "green" : "red";
      let style = ReactDOMRe.Style.make(~marginLeft="5px", ~color, ~fontWeight="bold", ());
      <span style> {valid ? rs({js|✓|js}) : rs({js|⚠|js})} </span>;
    };

    <>
      <h1> {rs("Taxes in NYC")} </h1>
      <div className="purpose">
        {rs("Calculate total U.S. taxes owed by an unmarried resident of NYC in ")}
        <select id="taxYearSelector" value=yearS onChange={eventS(setYearS)}>
          {ReasonReact.array(
             Array.map(
               year => <option value=year key=year> {rs(year)} </option>,
               Array.map(year => string_of_int(year), TaxRates.availableYears),
             ),
           )}
        </select>
      </div>
      <form id="input_form" method="get">
        <fieldset>
          <legend> {rs({js|⛃|js})} </legend>
          <label htmlFor="agi"> {rs("Adjusted Gross Income:")} </label>
          {rs("$ ")}
          <input name="income" type_="text" size=20 value=incomeS onChange={eventS(setIncomeS)} />
          {checkSign(checkIncome(formParams.income))}
          <br />
          <label htmlFor="itd"> {rs("Tax Deductions Total Amount: ")} </label>
          {rs("$ ")}
          <input name="deductions" type_="text" size=20 value=deductionsS onChange={eventS(setDeductionsS)} />
          {checkSign(checkDeductions(formParams.deductions))}
          <br />
          <label htmlFor="npe"> {rs("Number of Personal Exemptions: ")} </label>
          {rs("n ")}
          <input name="exemptions" type_="number" size=20 value=exemptionsS onChange={eventS(setExemptionsS)} />
          {checkSign(checkExemptions(formParams.exemptions))}
          <br />
          <input className="calc_submit" type_="button" value="Calculate" onClick=onCalc disabled={!isCalcEnabled} />
        </fieldset>
      </form>
      <br />
      <TaxReport params />
      <br />
    </>;
  };
};

fixUrlIfNecessary({
  // Default parameters:
  year: TaxRates.availableYears[Array.length(TaxRates.availableYears) - 1],
  income: 100000.0,
  deductions: 0.0,
  exemptions: 1,
});

let render = () => {
  ReactDOMRe.renderToElementWithId(<TaxAnalyzer />, "root");
};